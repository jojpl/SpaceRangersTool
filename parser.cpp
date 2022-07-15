#include "parser.h"

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

std::pair<std::string_view, std::string_view>
split_to_kv(std::string_view line)
{
	const auto f = line.find('=');
	if (f != line.npos)
	{
		return
		{
			line.substr(0, f),
			line.substr(f + 1)
		};
	}
	return {};
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

void validate(const std::string& mem)
{
	size_t lines = 0;
	size_t cnt_open = 0;
	size_t cnt_close = 0;

	auto get_statictic = [&lines, &cnt_open, &cnt_close](char ch)
	{
		if(ch == crlf_tag[0]) lines++;
		else if(ch == '{') cnt_open++;
		else if(ch == '}') cnt_close++;
	};
	// не парясь особо

	std::for_each(mem.begin(), mem.end(), get_statictic);

	if(cnt_open!= cnt_close)
		throw std::logic_error("mismatch {}");
}

void parse(const std::string& mem)
{
	validate(mem);
	throw std::logic_error("mismatch {}");
	
	Parser p;
	p.parse(mem);
	return;
}

void Parser::parse(const std::string& mem)
{
	init_ctx(mem);

	while (ctx.getline())
	{
		parse_line();
		int i = 7;
	}
	std::cout << out->Player->Name << std::endl;
}

void Parser::init_ctx(std::string_view mem)
{
	out = new Entities::Global{};
	ctx.stack.push({ out });

	ctx.tail_ = mem;
}

void Parser::parse_line()
{
	auto& s = ctx.stack.top();
	std::visit(Handler{ctx}, s);
}

bool Parser_Ctx::getline()
{
	if (!::getline(tail_, line_, tail_))
		return false;
	
	trim_tabs(line_);
	return true;
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
		return line_.substr(0, f);
	}
	return {};
}

std::pair<std::string_view, std::string_view>
Parser_Ctx::get_kv() const
{
	return split_to_kv(line_);
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
			PARSE_TO(Money)
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

std::string_view get_IType_use_lookup_ahead(Parser_Ctx& ctx)
{
	std::string_view out_line, out_next = ctx.tail_;
	while (getline(out_next, out_line, out_next))
	{
		trim_tabs(out_line);
		auto [k, v] = split_to_kv(out_line);

	}
	return {};
}

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
