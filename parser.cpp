#include "parser.h"
#include "convert.h"
#include "factory.hpp"

#include <io.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string_view>
#include <sstream>
#include <map>
#include <type_traits>

static const std::string open_tag  = " ^{";
static const std::string close_tag = "}";
static const std::string kv_delim_tag = "=";
static const std::string crlf_tag = "\r\n";

// MCVS
#ifdef _MSC_VER9
#define Starts_with(sw, example) (sw._Starts_with(example))
#else
#define Starts_with(sw, example) (sw.rfind(example, 0) == 0)
#endif

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

bool read_file(std::string& out, const std::string& path)
{
	std::ifstream f(path, std::ifstream::binary);
	if(!f) return false;

	f.seekg(0, std::ifstream::end);
	auto len = (size_t) f.tellg();
	f.seekg(0);

	std::vector<char> buf;
	buf.resize(len);

	f.read(buf.data(), len);

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
	//storage::clear_storage();
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
	out = Factory<Entities::Global>::create();
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

bool Parser_Ctx::is_object_kv() const
{
	return line_.find(kv_delim_tag) != line_.npos;
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


void Handler::on_new_obj(Entities::Global* p, std::string_view obj_name)
{
	if (obj_name == "Player")
	{
		p->Player = Factory<Entities::Player>::create();
		ctx.stack.push({ p->Player });
	}
	else if (obj_name == "StarList")
	{
		ctx.stack.push({ &p->StarList });
	}
}

void Handler::on_kv(Entities::Global* p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(Global)
		PARSE_TO(IDay)
	END_PARSE()
}

void Handler::on_new_obj(Entities::Player * p, std::string_view obj_name)
{
	if (obj_name == "EqList")
	{
		ctx.stack.push({ &p->EqList });
	}
}

void Handler::on_kv(Entities::Player * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(Player)
		PARSE_TO(ICurStarId)
		PARSE_TO(IFullName)
		PARSE_TO(IType)
		PARSE_TO(Name)
		PARSE_TO(IPlanet)
		PARSE_TO(Money)
		PARSE_TO(Goods)

		PARSE_TO(Debt)
	END_PARSE()
}

void Handler::on_new_obj(Entities::StarList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "StarId"))
	{
		int id = conv::extractId(obj_name);
		auto* star = Factory<Entities::Star>::create();
		star->Id = id;
		p->list.push_back(star);

		ctx.stack.push({ star });
	}
}

void Handler::on_new_obj(Entities::Star * p, std::string_view obj_name)
{
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
}

void Handler::on_kv(Entities::Star * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(Star)
		PARSE_TO(StarName)
		PARSE_TO(X)
		PARSE_TO(Y)
		PARSE_TO(Owners)
	END_PARSE()
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

void Handler::on_new_obj(Entities::EqList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx);

		auto* item = Factory<Entities::Item>::create(id);
		p->list.push_back(item);

		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(Entities::ShipList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ShipId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx);
		auto* item = Factory<Entities::Ship>::create();
		item->Id = id;
		p->list.push_back(item);

		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(Entities::Ship * p, std::string_view obj_name)
{
	if (obj_name == "EqList")
	{
		ctx.stack.push({ &p->EqList });
	}
}

void Handler::on_kv(Entities::Ship * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(Ship)
		PARSE_TO(IFullName)
		PARSE_TO(IType)
		PARSE_TO(Name)
		PARSE_TO(IPlanet)
		PARSE_TO(Goods)
		PARSE_TO(Money)
	END_PARSE()
}

void Handler::on_new_obj(Entities::PlanetList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "PlanetId"))
	{
		int id = conv::extractId(obj_name);

		auto* item = Factory<Entities::Planet>::create(id);
		item->Id = id;
		p->list.push_back(item);

		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(Entities::Planet * p, std::string_view obj_name)
{
	if (obj_name == "EqShop")
	{
		ctx.stack.push({ &p->EqShop });
	}
	else if (obj_name == "Treasure")
	{
		ctx.stack.push({ &p->Treasure });
	}
}

void Handler::on_kv(Entities::Planet * p, std::string_view key, std::string_view value)
{
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

void Handler::on_new_obj(Entities::Junk * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx);

		auto* item = Factory<Entities::Item>::create(id);
		p->list.push_back(item);

		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(Entities::EqShop * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx);

		auto* item = Factory<Entities::Item>::create(id);;
		p->list.push_back(item);

		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(Entities::Treasure * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "HiddenItem"))
	{
		//int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx);
		auto* item = Factory<Entities::HiddenItem>::create();
		p->list.push_back(item);

		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(Entities::HiddenItem * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx);
		auto* item = Factory<Entities::Item>::create(id);
		p->item = item;

		ctx.stack.push({ item });
	}
}

void Handler::on_kv(Entities::HiddenItem * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(HiddenItem)
		PARSE_TO(LandType)
		PARSE_TO(Depth)
	END_PARSE()
}

void Handler::on_kv(Entities::Item * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(Item)
		PARSE_TO(IName)
		PARSE_TO(IType)
		PARSE_TO(Size)
		PARSE_TO(Ammo)
		PARSE_TO(Armor)
		PARSE_TO(Bonus)
		PARSE_TO(Broken)
		PARSE_TO(BuiltByPirate)
		PARSE_TO(Capacity)
		PARSE_TO(Cost)
		PARSE_TO(DomSeries)
		PARSE_TO(Durability)
		PARSE_TO(Fuel)
		PARSE_TO(IBonusName)
		PARSE_TO(ISeriesName)
		PARSE_TO(ISpecialName)
		PARSE_TO(Jump)
		PARSE_TO(MaxAmmo)
		PARSE_TO(MaxDamage)
		PARSE_TO(MinDamage)
		PARSE_TO(NoDrop)
		PARSE_TO(Owner)
		PARSE_TO(Power)
		PARSE_TO(Radius)
		PARSE_TO(Repair)
		PARSE_TO(Series)
		PARSE_TO(ShipType)
		PARSE_TO(Special)
		PARSE_TO(Speed)
		PARSE_TO(SpeedMax)
		PARSE_TO(SpeedMin)
		PARSE_TO(SysName)
		PARSE_TO(TechLevel)
		PARSE_TO(X)
		PARSE_TO(Y)
	END_PARSE()
}
