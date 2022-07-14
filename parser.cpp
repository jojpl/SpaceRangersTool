#include "parser.h"

#include "Entities.h"
#include "convert.h"

#include <io.h>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <sstream>
#include <map>
#include <type_traits>

static const std::string open_tag  = " ^{";
static const std::string close_tag = "}";
static const std::string crlf_tag = "\r\n";

#define BEGIN_PARSE_FOR(struct_name) { using t = struct_name; do {if(false){}
#define PARSE_TO(field) else if(conv::parse(&t::field, p, key, value)) break;
#define END_PARSE() }while(false);}

bool read_file(std::string& out, const std::wstring& path)
{
	FILE* f = _wfopen(path.c_str(), L"rb");
	if (!f) return false;
	long len = _filelength(_fileno(f));

	std::vector<char> buf;
	buf.resize(len);
	fread(buf.data(), len, 1, f);
	fclose(f);
	out.assign(buf.begin(), buf.end());
	return !out.empty();
}

bool getline(std::string::const_iterator beg,
			std::string::const_iterator end,
			std::string::const_iterator& line_beg,
			std::string::const_iterator& line_end,
			std::string::const_iterator& next_beg)
{
	const auto f = std::search(beg, end, crlf_tag.cbegin(), crlf_tag.cend());
	if (f == end)
		return false;

	line_beg = beg;
	line_end = f;
	next_beg = f + crlf_tag.size();
	return true;
}

void trim_tabs(std::string::const_iterator& beg,
	std::string::const_iterator& end)
{
	while (beg !=end && *beg =='\t') ++beg;
	while (beg !=end && *(end - 1) == '\t') --end;
}

void trim_tabs(std::string_view& beg)
{
	while (beg.size() && beg.front() == '\t') beg.remove_prefix(1);
	while (beg.size() && beg.back() == '\t') beg.remove_suffix(1);
}

void parse(const std::string& mem)
{
	size_t lines = std::count(mem.begin(), mem.end(), '\n');
	size_t cnt_open = std::count(mem.begin(), mem.end(), '{');
	size_t cnt_close = std::count(mem.begin(), mem.end(), '}');

	if(cnt_open!= cnt_close)
		throw std::logic_error("mismatch {}");

	Parser p;
	p.parse(mem);
	return;
}

bool getline(	std::string_view str,
				std::string_view& out_line,
				std::string_view& out_next)
{
	if (str.data())
	{
		auto f = str.find(crlf_tag);
		if (f != std::string_view::npos)
		{
			out_line = str.substr(0, f);
			out_next = str.substr(f + crlf_tag.size());
			return true;
		}

		out_line = str;
		out_next = {};
		return true;
	}
	return false;
}

void Parser::parse(const std::string& mem)
{
	ctx.init(mem);

	std::string_view line, next_line{ mem };

	while (getline(next_line, line, next_line))
	{
		trim_tabs(line);

		ctx.set_line(line);

		parse_line();
		int i =7;
	}
}

void Parser::parse_line()
{
	// resolve current struct
	// save(push) it to ctx
	// Идея в том, чтоб не реккурсивно идти - один проход одно заполнение
	// чего из контекста выбирать, диспатчить по нему,
	// в хендлерах для конкретной структуры заполнять поля
	// при встрече новой структуры менять контекст (+ создание) и возвращаться
	//std::string out;
	//out.assign(ctx.line_beg_, ctx.line_end_);

	auto& s = ctx.stack.top();
	std::visit(Handler{ctx}, s);
	return;
}


void Parser_Ctx::init(const std::string& mem)
{
	out         = new Entities::Global{};
	stack.push({out});
}

void Parser_Ctx::set_line(std::string_view line_beg)
{
	line_ = line_beg;
}

bool Parser_Ctx::is_object_open() const
{
	return line_.find(open_tag)!= line_.npos;
}

bool Parser_Ctx::is_object_close() const
{
	return line_.find(close_tag) != line_.npos;
}

std::string_view 
Parser_Ctx::get_object_name() const
{
	const auto f = line_.find(open_tag);
	if (f != line_.npos)
	{
		return line_.substr(0, f + 1);
	}
	return {};
}

std::pair<std::string_view, std::string_view>
Parser_Ctx::get_kv() const
{
	const auto f = line_.find('=');
	if (f != line_.npos)
	{
		return 
		{
			line_.substr(0, f),
			line_.substr(f + 1)
		};
	}
	return {};
}

// unknown type correct handle
void Handler::operator()(Entities::Unknown*)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
}

void Handler::operator()(Entities::Global* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		if (obj_name == "Player")
		{
			p->Player = new Entities::Player{};
			ctx.stack.push({p->Player});
		}
		//else if (obj_name == "StarList")
		//{
		//	ctx.stack.push({ &p->StarList });
		//}
		else
		{
			ctx.stack.push({ (Entities::Unknown*)nullptr });
		}
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		const auto[key, value] = ctx.get_kv();

		using namespace Entities;
		BEGIN_PARSE_FOR(Global)
			PARSE_TO(IDay)
		END_PARSE()
	}
}

void Handler::operator()(Entities::Player* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		if (obj_name == "EqList")
		{
			ctx.stack.push({ &p->EqList });
		}
		else
		{
			ctx.stack.push({ (Entities::Unknown*)nullptr });
		}
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		const auto[key, value] = ctx.get_kv();

		using namespace Entities;
		BEGIN_PARSE_FOR(Player)
			PARSE_TO(ICurStarId)
			PARSE_TO(IFullName)
			PARSE_TO(IType)
			PARSE_TO(Name)
			PARSE_TO(IPlanet)
			PARSE_TO(Goods)
		END_PARSE()
	}
}

//void Handler::operator()(Entities::StarList* p)
//{
//
//}
//
//void Handler::operator()(Entities::Star* p)
//{
//
//}

void Handler::operator()(Entities::EqList* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		auto pos = obj_name.find("Item");
		if (pos!=obj_name.npos)
		{
			int id = conv::extractId(obj_name);
			auto* item = new Entities::Item{};
			item->id = id;
			p->list.push_back(item);

			ctx.stack.push({ item });
		}
		else
		{
			ctx.stack.push({ (Entities::Unknown*)nullptr });
		}
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
}

void Handler::operator()(Entities::Item* p)
{
	if (ctx.is_object_open())
	{
		ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		const auto[key, value] = ctx.get_kv();
		
		using namespace Entities;
		BEGIN_PARSE_FOR(Item)
			PARSE_TO(IName)
			PARSE_TO(IType)
			PARSE_TO(Owner)
			PARSE_TO(Size)
			PARSE_TO(Cost)
			PARSE_TO(NoDrop)
			PARSE_TO(Special)
			PARSE_TO(ISpecialName)
			PARSE_TO(DomSeries)
			PARSE_TO(TechLevel)
			PARSE_TO(Armor)
			PARSE_TO(ShipType)
			PARSE_TO(Series)
			PARSE_TO(ISeriesName)
			PARSE_TO(BuiltByPirate)
		END_PARSE()
	}
}
