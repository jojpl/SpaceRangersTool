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
	while (beg !=end && *beg == '\t') ++beg;
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
	
	Parser p;
	p.parse(mem);
	return;
}

void Parser::parse(const std::string& mem)
{
	init_ctx(mem);

	while (ctx.getline())
		parse_line();

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
		else if (obj_name == "StarList")
		{
			ctx.stack.push({ &p->StarList });
		}
		else
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
		//if (obj_name == "ArtsList")
		//{
		//	ctx.stack.push({ &p->ArtsList });
		//}
		else
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

void Handler::operator()(Entities::StarList* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		auto pos = obj_name.find("StarId");
		if (pos != obj_name.npos)
		{
			int id = conv::extractId(obj_name);
			auto* star = new Entities::Star{};
			star->Id = id;
			p->list.push_back(star);

			ctx.stack.push({ star });
		}
		else
			ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
}

void Handler::operator()(Entities::Star* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		if (obj_name == "ShipList")
		{
			ctx.stack.push({ &p->ShipList });
		}
		else if (obj_name == "PlanetList")
		{
			ctx.stack.push({ &p->PlanetList });
		}		
		else if (obj_name == "Junk")
		{
			ctx.stack.push({ &p->Junk });
		}
		else
			ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		//int Id;
		//string StarName;
		//double X;
		//double Y;
		//string Owners;
		//ShipList ShipList;
		//PlanetList PlanetList;
		//Junk Junk;
	}
}

std::string_view get_IType_use_lookup_ahead(Parser_Ctx& ctx)
{
	// use Parser_Ctx is more correct
	std::string_view out_line, out_next = ctx.tail_;
	int findwidth = 10;
	while (getline(out_next, out_line, out_next) && findwidth--)
	{
		trim_tabs(out_line);
		auto [k, v] = split_to_kv(out_line);
		if(k == "IType")
			return v;
	}
	return {};
}

void Handler::operator()(Entities::EqList* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		auto pos = obj_name.find("ItemId");
		if (pos==0)
		{
			int id = conv::extractId(obj_name);
			auto IType = get_IType_use_lookup_ahead(ctx);
			auto* item = new Entities::Item{};
			item->Id = id;
			p->list.push_back(item);

			ctx.stack.push({ item });
		}
		else
			ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
}

void Handler::operator()(Entities::ShipList * p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		auto pos = obj_name.find("ShipId");
		if (pos == 0)
		{
			int id = conv::extractId(obj_name);
			auto IType = get_IType_use_lookup_ahead(ctx);
			auto* item = new Entities::Ship{};
			item->Id = id;
			p->list.push_back(item);

			ctx.stack.push({ item });
		}
		else
			ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
}

void Handler::operator()(Entities::Ship * p)
{
	default_impl(p);
}

void Handler::operator()(Entities::PlanetList * p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		size_t pos = obj_name.find("PlanetId");
		if (pos == 0)
		{
			int id = conv::extractId(obj_name);

			auto* item = new Entities::Planet{};
			item->Id = id;
			p->list.push_back(item);

			ctx.stack.push({ item });
		}
		else
			ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
}

void Handler::operator()(Entities::Planet * p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		if (obj_name == "EqShop")
		{
			ctx.stack.push({ &p->EqShop });
		}
		else if (obj_name == "Treasure")
		{
			ctx.stack.push({ &p->Treasure });
		}
		else
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
		BEGIN_PARSE_FOR(Planet)
			PARSE_TO(PlanetName)
			PARSE_TO(Owner)
			PARSE_TO(Race)
			PARSE_TO(Economy)
			PARSE_TO(Goverment)
			PARSE_TO(ISize)
			PARSE_TO(OrbitRadius)
			PARSE_TO(OrbitAngle)
			PARSE_TO(RelationToPlayer)
			PARSE_TO(IMainTechLevel)
			PARSE_TO(CurrentInvention)
			PARSE_TO(CurrentInventionPoints)
			PARSE_TO(ShopGoods)
			PARSE_TO(ShopGoodsSale)
			PARSE_TO(ShopGoodsBuy)
		END_PARSE()
	}
}

void Handler::operator()(Entities::Junk * p)
{
	default_impl(p);
}

void Handler::operator()(Entities::EqShop * p)
{
	default_impl(p);
}

void Handler::operator()(Entities::Treasure * p)
{
	default_impl(p);
}

void Handler::operator()(Entities::Item* p)
{
	if (ctx.is_object_open())
	{
		//terminal
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
			PARSE_TO(X)
			PARSE_TO(Y)
		END_PARSE()
	}
}

void Handler::default_impl(void* p)
{
	this->operator()( (Entities::Unknown*)p );
}
