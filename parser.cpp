#include "parser.h"
#include "convert.h"
#include "factory.hpp"
#include "performance_tracker.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string_view>
#include <type_traits>

#include <boost/algorithm/string/predicate.hpp>

namespace parser
{

static const std::string open_tag  = " ^{";
static const std::string close_tag = "}";
static const std::string kv_delim_tag = "=";
static const std::string crlf_tag = "\r\n";

#define Starts_with(sw, example) (boost::starts_with(sw, example))

#define BEGIN_PARSE_FOR(struct_name) { using t = struct_name; do {if(false){}
#define PARSE_TO(field) else if(conv::parse(&t::field, p, key, value)) break;
#define END_PARSE() }while(false);}

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
	struct statistic
	{
		size_t cnt_open = 0;
		size_t cnt_close = 0;

		int cnt_open_tmp = 0;
		int cnt_close_tmp = 0;

		int cnt_open_tag_size = open_tag.size();
		int cnt_close_tag_size = close_tag.size();
	} st;

	auto get_statictic = [&st](char ch)
	{
		if(ch == open_tag[st.cnt_open_tmp])
		{
			st.cnt_open_tmp++;
			if (st.cnt_open_tmp == st.cnt_open_tag_size)
			{
				st.cnt_open++;
				st.cnt_open_tmp = 0;
			}
			st.cnt_close_tmp = 0;
		}
		else if(ch == close_tag[st.cnt_close_tmp])
		{
			st.cnt_close_tmp++;
			if (st.cnt_close_tmp == st.cnt_close_tag_size)
			{
				st.cnt_close++;
				st.cnt_close_tmp = 0;
			}
			st.cnt_open_tmp = 0;
		}
		else
		{
			st.cnt_open_tmp = 0;
			st.cnt_close_tmp = 0;
		}
	};

	std::for_each(mem.begin(), mem.end(), get_statictic);

	if(st.cnt_open!= st.cnt_close)
		throw std::logic_error("mismatch {}");
}

Global* parse(const std::string& mem)
{
	performance_tracker tr(__FUNCTION__);
	validate(mem);
	
	Parser p;
	p.parse(mem);
	//storage::Registrator::clear_storage();
	return p.get_parsed();
}

void Parser::parse(const std::string& mem)
{
	init_ctx(mem);

	while (ctx.getline())
		parse_line();
}

Global * Parser::get_parsed()
{
	return out;
}

void Parser::init_ctx(std::string_view mem)
{
	ctx.tail_ = mem;
	out       = Factory<Global>::create();
	ctx.stack.push({ out });
}

void Parser::parse_line()
{
	auto& s = ctx.stack.top();
	std::visit(Handler{ctx}, s);
}

bool Parser_Ctx::getline()
{
	if (!parser::getline(tail_, line_, tail_))
		return false;
	
	trim_tabs(line_);
	return true;
}

bool Parser_Ctx::is_object_open() const
{
	return boost::ends_with(line_, open_tag);
	//return line_.find(open_tag)!= line_.npos;
}

bool Parser_Ctx::is_object_close() const
{
	return boost::ends_with(line_, close_tag);
	//return line_.find(close_tag) != line_.npos;
}

bool Parser_Ctx::is_object_kv() const
{
	return line_.find(kv_delim_tag) != line_.npos;
}

std::string_view 
Parser_Ctx::get_object_name() const
{
	//const auto f = line_.find(open_tag);
	const auto f = line_.rfind(open_tag);
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


void Handler::on_new_obj(Global* p, std::string_view obj_name)
{
	if (obj_name == "Player")
	{
		p->Player = Factory<Player>::create(0, ctx.location_);
		ctx.stack.push({ p->Player });
	}
	else if (obj_name == "StarList")
	{
		ctx.stack.push({ &p->StarList });
	}
}

void Handler::on_kv(Global* p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(Global)
		PARSE_TO(IDay)
	END_PARSE()
}

void Handler::on_new_obj(Player * p, std::string_view obj_name)
{
	if (obj_name == "EqList")
	{
		ctx.stack.push({ &p->EqList });
	}
	else if (obj_name == "ArtsList")
	{
		ctx.stack.push({ &p->ArtsList });
	}
}

void Handler::on_kv(Player * p, std::string_view key, std::string_view value)
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

void Handler::on_new_obj(StarList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "StarId"))
	{
		int id = conv::extractId(obj_name);
		auto* star = Factory<Star>::create(id);
		p->list.push_back(star);
		ctx.location_.star = star;
		ctx.stack.push({ star });
	}
}

void Handler::on_new_obj(Star * p, std::string_view obj_name)
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

void Handler::on_kv(Star * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(Star)
		PARSE_TO(StarName)
		PARSE_TO(X)
		PARSE_TO(Y)
		PARSE_TO(Owners)
	END_PARSE()
}

void Handler::on_close_obj(Star * p)
{
	ctx.location_.star = nullptr;
}

Type get_IType_use_lookup_ahead(std::string_view tail)
{
	// use Parser_Ctx is more correct
	std::string_view out_line, out_next = tail;
	
	Type t{};
	int findwidth = 10;
	while (getline(out_next, out_line, out_next) && findwidth--)
	{
		trim_tabs(out_line);
		auto [k, v] = split_to_kv(out_line);
		if (k == "IType")
		{
			conv::from_string(t, v);
			return t;
		}
	}
	return t;
}

void Handler::on_new_obj(EqList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);

		auto* item = Factory<Item>::create(id, ctx.location_);
		p->list.push_back(item);
		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(ArtsList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);

		auto* item = Factory<Item>::create(id, ctx.location_);
		p->list.push_back(item);
		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(ShipList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ShipId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);
		if (std::find(cbegin(ShipBases::allowedTypes),
					cend(ShipBases::allowedTypes),
					IType) != cend(ShipBases::allowedTypes))
		{
			auto* item = Factory<ShipBases>::create(id, ctx.location_);
			p->list.push_back(item);
			ctx.stack.push({ item });
		}
		else {
			auto* item = Factory<Ship>::create(id, ctx.location_);
			p->list.push_back(item);
			ctx.stack.push({ item });
		}
	}
}

void Handler::on_new_obj(Ship * p, std::string_view obj_name)
{
	if (obj_name == "EqList")
	{
		ctx.stack.push({ &p->EqList });
	}
	else if (obj_name == "ArtsList")
	{
		ctx.stack.push({ &p->ArtsList });
	}
}

void Handler::on_kv(Ship * p, std::string_view key, std::string_view value)
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

void Handler::on_new_obj(ShipBases * p, std::string_view obj_name)
{
	if (obj_name == "EqList")
	{
		ctx.stack.push({ &p->EqList });
	}
	else if (obj_name == "EqShop")
	{
		ctx.stack.push({ &p->EqShop });
	}
}

void Handler::on_kv(ShipBases * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(ShipBases)
		PARSE_TO(IFullName)
		PARSE_TO(IType)
		PARSE_TO(Name)
		PARSE_TO(ShopGoods)
		PARSE_TO(ShopGoodsSale)
		PARSE_TO(ShopGoodsBuy)
	END_PARSE()
}

void Handler::on_new_obj(PlanetList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "PlanetId"))
	{
		int id = conv::extractId(obj_name);

		auto* planet = Factory<Planet>::create(id, ctx.location_);
		p->list.push_back(planet);
		planet->location.planet = planet; //fix it
		ctx.location_.planet = planet;
		ctx.stack.push({ planet });
	}
}

void Handler::on_new_obj(Planet * p, std::string_view obj_name)
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

void Handler::on_kv(Planet * p, std::string_view key, std::string_view value)
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

void Handler::on_close_obj(Planet * p)
{
	ctx.location_.planet = nullptr;
}

void Handler::on_new_obj(Junk * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);

		auto* item = Factory<Item>::create(id, ctx.location_);
		p->list.push_back(item);
		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(EqShop * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);

		auto* item = Factory<Item>::create(id, ctx.location_);;
		p->list.push_back(item);
		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(Treasure * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "HiddenItem"))
	{
		//int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);
		auto* item = Factory<HiddenItem>::create();
		p->list.push_back(item);
		ctx.stack.push({ item });
	}
}

void Handler::on_new_obj(HiddenItem * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);
		auto* item = Factory<Item>::create(id, ctx.location_);
		p->item = item;
		ctx.stack.push({ item });
	}
}

void Handler::on_kv(HiddenItem * p, std::string_view key, std::string_view value)
{
	using namespace Entities;
	BEGIN_PARSE_FOR(HiddenItem)
		PARSE_TO(LandType)
		PARSE_TO(Depth)
	END_PARSE()
}

void Handler::on_kv(Item * p, std::string_view key, std::string_view value)
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

} // namespace parser
