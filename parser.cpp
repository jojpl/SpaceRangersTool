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

#include <set>
#include <ostream>
#include <fstream>
#include <sstream>

static const std::string open_tag  = " ^{";
static const std::string close_tag = "}";
static const std::string crlf_tag = "\r\n";

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

		//ctx.set_line(line_beg, line_end);
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
	std::visit(Handler{ctx}, s.p);
#if 0
	switch (s.st)
	{
		case Curstruct::Global:
			handle((Entities::Global*)s.p);
			break;
		case Curstruct::Player:
			handle((Entities::Player*)s.p);
			break;
		case Curstruct::EqList:
			handle((Entities::EqList*)s.p);
			break;		
		case Curstruct::Item:
			handle((Entities::Item*)s.p);
			break;
		case Curstruct::StarList:
			handle((Entities::StarList*)s.p);
			break;
		case Curstruct::Unknown:
			handle((void*)s.p);
		default:
			break;
	}
#endif
	return;
}

#if 0
void Parser::handle(Entities::Global* p)
{
	//int IDay;
	//Player* Player;
	//StarList StarList;

	
}

void Parser::handle(Entities::Player* p)
{
	handle((void*)p);
}

void Parser::handle(Entities::EqList* p)
{
	handle((void*)p);
}

void Parser::handle(Entities::StarList * p)
{
	handle((void*)p);
}

void Parser::handle(Entities::Item* p)
{
	handle((void*)p);
}

void Parser::handle(void* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		ctx.stack.push({ nullptr, Curstruct::Unknown });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		auto[key, value] = ctx.get_kv();
		return;
	}
}
#endif //0

void Parser_Ctx::init(const std::string& mem)
{
	main_begin_ = cbegin(mem);
	main_end_   = cend(mem);

	out         = new Entities::Global{};
	stack.push({out});
}

void Parser_Ctx::set_line( std::string::const_iterator line_beg,
						   std::string::const_iterator line_end )
{
	line_beg_ = std::move(line_beg);
	line_end_ = std::move(line_end);
}

void Parser_Ctx::set_line(std::string_view line_beg)
{
	line_ = line_beg;
}

bool Parser_Ctx::is_object_open() const
{
	//return std::search(line_beg_, line_end_, open_tag.cbegin(), open_tag.cend()) != line_end_;
	return line_.find(open_tag)!= line_.npos;
}

bool Parser_Ctx::is_object_close() const
{
	//return std::search(line_beg_, line_end_, close_tag.cbegin(), close_tag.cend()) != line_end_;
	return line_.find(close_tag) != line_.npos;
}

std::string_view 
Parser_Ctx::get_object_name() const
{
	//const auto f = std::search(line_beg_, line_end_, open_tag.cbegin(), open_tag.cend());
	//if (f != line_end_)
	//{
	//	return std::string_view(&*line_beg_, std::distance(line_beg_, f));
	//}
	//return {};

	const auto f = line_.find(open_tag);
	if (f != line_.npos)
	{
		//return std::string_view(&*line_beg_, std::distance(line_beg_, f));
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
			//std::string_view(&*line_beg_, std::distance(line_beg_, f)),
			//std::string_view(&*value_beg, std::distance(value_beg, line_end_))
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

		//if (key == "IDay")
		//	p->IDay = conv::to_int(value);
		conv::parse(&Entities::Global::IDay, p, key, value);
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

		#define BEGIN_PARSE_FOR(struct_name) { using t = struct_name; do {if(false){}
		#define PARSE_TO(field) else if(conv::parse(&t::field, p, key, value)) break;
		#define END_PARSE() }while(false);}

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

void dump(std::ostream& os, std::map<std::string, std::set<std::string>>& obj)
{
	std::string op {"<"},		cl{ ">"}, 
				op_cl {"</"},	cl_cl{"/>"};
	for (auto& [IType, set] : obj)
	{
		os << 
			"<" << IType << ">"
		<< std::endl;

		for (auto& val : set)
		{
			os << 
				"\t" << "<" << val << "/>"
				<< std::endl;
		}


		os << 
			"</" << IType << ">"
		<< std::endl;
		os << std::endl;
	}
}

void dump_to_mem(std::map<std::string, std::set<std::string>>& obj)
{
	std::stringstream ss;
	dump(ss, obj);
}

void dump_to_file(std::map<std::string, std::set<std::string>>& obj)
{
	
	std::ofstream os("dump");
	dump(os, obj);
}

void some(const std::string& mem)
{
	std::string_view sw_mem{mem};
	{
		size_t p1 = 0;
		std::map<std::string, std::set<std::string>> cnt;
		while(true)
		{
			p1 = sw_mem.find("ItemId", p1);
			//p1 = sw_mem.find("ShipId", p1);
			if(p1 == std::string_view::npos)
				break;

			auto p2 = sw_mem.find(open_tag, p1);
			p2 = sw_mem.find(crlf_tag, p2);
			//std::string_view tmp2 = sw_mem.substr(p1, p2 - p1);
			p2+=2;


			auto p3 = sw_mem.find(close_tag, p2);
			p3 += close_tag.size();
			std::string_view tmp = sw_mem.substr(p2, p3 - p2);
			{
				std::set<std::string> set_keys;
				std::string_view val;
				size_t i1 = 0;
				do{
					i1 = tmp.find('=', i1);
					if(i1 == std::string_view::npos)
						break;
					int beg = i1;
					beg--;

					while (true)
					{
						if(isalnum(tmp[beg])) beg--;
						else
						{
							beg++;
							break;
						}
					}
					std::string_view key = tmp.substr(beg, i1 - beg);
					if (key == "IType")
					{
						auto val_beg_pos = i1 + 1;
						auto val_end_pos = tmp.find(crlf_tag, i1);
						val = tmp.substr(val_beg_pos, val_end_pos - val_beg_pos);
					}
					set_keys.emplace(key.cbegin(), key.cend());
					i1 = tmp.find(crlf_tag, i1);
				}
				while (i1!= std::string_view::npos);

				cnt[{val.cbegin(), val.cend()}] = std::move(set_keys);
			}
			p1 = p3;
		}
		dump_to_mem(cnt);
		return;
	}
	//while()
}