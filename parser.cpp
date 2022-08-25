#include "parser.h"
#include "convert.h"
#include "factory.hpp"
#include "performance_tracker.hpp"

#include <algorithm>
#include <map>
#include <string_view>

#include <boost/algorithm/string/predicate.hpp>

namespace parser
{

using namespace std::string_view_literals;

constexpr std::string_view open_tag     = " ^{";
constexpr std::string_view close_tag    = "}";
constexpr std::string_view kv_delim_tag = "=";
constexpr std::string_view crlf_tag     = "\r\n";

#define Starts_with(sw, example) (boost::starts_with(sw, example))

#define BEGIN_PARSE_FOR(struct_name) { do { if(false) {}
#define PARSE_TO(field) else if(constexpr auto field##sw = #field##sv; key == field##sw) \
							{\
								conv::from_string(p->field, value);\
								return true;\
							}
#define END_PARSE() }while(false);}

void trim_tabs(std::string_view& beg)
{
	while (beg.size() && beg.front() == '\t') beg.remove_prefix(1);
	while (beg.size() && beg.back() == '\t') beg.remove_suffix(1);
}

std::pair<std::string_view, std::string_view>
split_to_kv(std::string_view line)
{
	const auto f = line.find(kv_delim_tag);
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

	// skip for perf

	//std::for_each(mem.begin(), mem.end(), get_statictic);

	//if(st.cnt_open!= st.cnt_close)
	//	throw std::logic_error("mismatch {}");
	if (!boost::starts_with(mem, "FinalizationName="))
		throw std::logic_error("wrong format");
}

bool validate_impl(Global* out)
{
	if (!out) return false;
	if (!out->Player) return false;
	if (out->StarList.list.empty()) return false;
	
	return true;
}

void validate_parsed(Global* out)
{
	if(!validate_impl(out)) throw std::logic_error("Wrong parse!");
}

Global* parse(const std::string& mem)
{
	performance_tracker tr(__FUNCTION__);
	
	Parser p;
	p.parse(mem);
	//storage::Registrator::clear_storage();
	return p.get_parsed();
}

void Parser::parse(const std::string& mem)
{
	validate(mem);

	init_ctx(mem);

	while (ctx.getline())
		parse_line();

	validate_parsed(out_);

	fix_skiped_look_forwarded_options();
}

Global * Parser::get_parsed()
{
	return out_;
}

void Parser::init_ctx(std::string_view mem)
{
	ctx.tail_ = mem;
	out_       = storage::Factory<Global>::create();
	ctx.stack.push({ out_ });
}

void Parser::fix_skiped_look_forwarded_options()
{
	for(auto& i : out_->HoleList.list) 
	{
		i->from.star = storage::find_star_by_id(i->Star1Id);
		i->to.star = storage::find_star_by_id(i->Star2Id);
	}

	auto* player = out_->Player;
	
	player->location.star =
		storage::find_star_by_id(player->ICurStarId);
	player->location.planet =
		storage::find_planet_by_name(player->IPlanet);

	for(auto& i : player->ArtsList.list) i->location = player->location;
	for(auto& i : player->EqList.list) i->location = player->location;
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

bool Handler::on_new_obj(Global* p, std::string_view obj_name)
{
	if (obj_name == "Player")
	{
		p->Player = storage::Factory<Player>::create(0, ctx.location_);
		ctx.stack.push({ p->Player });
		return true;
	}
	else if (obj_name == "StarList")
	{
		ctx.stack.push({ &p->StarList });
		return true;
	}
	else if (obj_name == "HoleList")
	{
		ctx.stack.push({ &p->HoleList });
		return true;
	}
	return false;
}

bool Handler::on_kv(Global* p, std::string_view key, std::string_view value)
{
	BEGIN_PARSE_FOR(Global)
		PARSE_TO(IDay)
	END_PARSE()

	return false;
}

bool Handler::on_new_obj(Player * p, std::string_view obj_name)
{
	return on_new_obj((Player::Inherit*)p, obj_name);
}

bool Handler::on_kv(Player * p, std::string_view key, std::string_view value)
{
	BEGIN_PARSE_FOR(Player)
		PARSE_TO(ICurStarId)
		PARSE_TO(Debt)
	END_PARSE()

	return on_kv((Player::Inherit*)p, key, value);
}

bool Handler::on_new_obj(StarList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "StarId"))
	{
		int id = conv::extractId(obj_name);
		auto* star = storage::Factory<Star>::create(id);
		p->list.push_back(star);
		ctx.location_.star = star;
		ctx.stack.push({ star });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(Star * p, std::string_view obj_name)
{
	if (obj_name == "ShipList")
	{
		ctx.stack.push({ &p->ShipList });
		return true;
	}
	else if (obj_name == "PlanetList")
	{
		ctx.stack.push({ &p->PlanetList });
		return true;
	}
	else if (obj_name == "Junk")
	{
		ctx.stack.push({ &p->Junk });
		return true;
	}
	return false;
}

bool Handler::on_kv(Star * p, std::string_view key, std::string_view value)
{
	BEGIN_PARSE_FOR(Star)
		PARSE_TO(StarName)
		PARSE_TO(X)
		PARSE_TO(Y)
		PARSE_TO(Owners)
	END_PARSE()

	return false;
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
	throw std::logic_error(__FUNCTION__);
}

bool Handler::on_new_obj(EqList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);

		auto* item = storage::Factory<Item>::create(id, ctx.location_);
		p->list.push_back(item);
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(ArtsList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);

		auto* item = storage::Factory<Item>::create(id, ctx.location_);
		p->list.push_back(item);
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(ShipList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ShipId"))
	{
		int id = conv::extractId(obj_name);
		auto IType = get_IType_use_lookup_ahead(ctx.tail_);
		if (std::find(cbegin(Station::allowedTypes),
					cend(Station::allowedTypes),
					IType) != cend(Station::allowedTypes))
		{
			auto* item = storage::Factory<Station>::create(id, ctx.location_);
			p->list.push_back(item);
			ctx.stack.push({ item });
			return true;
		}
		else {
			auto* item = storage::Factory<Ship>::create(id, ctx.location_);
			p->list.push_back(item);
			ctx.stack.push({ item });
			return true;
		}
	}
	return false;
}

bool Handler::on_new_obj(Ship * p, std::string_view obj_name)
{
	if (obj_name == "EqList")
	{
		ctx.stack.push({ &p->EqList });
		return true;
	}
	else if (obj_name == "ArtsList")
	{
		ctx.stack.push({ &p->ArtsList });
		return true;
	}
	return false;
}

bool Handler::on_kv(Ship * p, std::string_view key, std::string_view value)
{
	BEGIN_PARSE_FOR(Ship)
		PARSE_TO(IFullName)
		PARSE_TO(IType)
		PARSE_TO(Name)
		PARSE_TO(IPlanet)
		PARSE_TO(Goods)
		PARSE_TO(Money)
	END_PARSE()

	return false;
}

bool Handler::on_new_obj(Station * p, std::string_view obj_name)
{
	if (obj_name == "EqList")
	{
		ctx.stack.push({ &p->EqList });
		return true;
	}
	else if (obj_name == "EqShop")
	{
		ctx.stack.push({ &p->EqShop });
		return true;
	}
	return on_new_obj((Station::Inherit*)p, obj_name);
}

bool Handler::on_kv(Station * p, std::string_view key, std::string_view value)
{
	BEGIN_PARSE_FOR(Station)
		PARSE_TO(ShopGoods)
		PARSE_TO(ShopGoodsSale)
		PARSE_TO(ShopGoodsBuy)
	END_PARSE()
	
	return false;
}

void Handler::on_close_obj(Station * p)
{
	auto* obj = storage::Factory<ObjPrices>::create();
	obj->buy  = p->ShopGoodsBuy;
	obj->sale = p->ShopGoodsSale;
	obj->qty  = p->ShopGoods;
	obj->location = p->location;
	obj->shipshop = p;
}

bool Handler::on_new_obj(PlanetList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "PlanetId"))
	{
		int id = conv::extractId(obj_name);

		auto* planet = storage::Factory<Planet>::create(id, ctx.location_);
		p->list.push_back(planet);
		planet->location.planet = planet; //fix it
		ctx.location_.planet = planet;
		ctx.stack.push({ planet });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(Planet * p, std::string_view obj_name)
{
	if (obj_name == "EqShop")
	{
		ctx.stack.push({ &p->EqShop });
		return true;
	}
	else if (obj_name == "Treasure")
	{
		ctx.stack.push({ &p->Treasure });
		return true;
	}
	else if (obj_name == "Garrison")
	{
		ctx.stack.push({ &p->Garrison });
		return true;
	}
	return false;
}

bool Handler::on_kv(Planet * p, std::string_view key, std::string_view value)
{
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
	
	return false;
}

void Handler::on_close_obj(Planet * p)
{
	ctx.location_.planet = nullptr;

	auto* obj = storage::Factory<ObjPrices>::create();
	obj->buy  = p->ShopGoodsBuy;
	obj->sale = p->ShopGoodsSale;
	obj->qty  = p->ShopGoods;
	obj->location = p->location;
}

bool Handler::on_new_obj(Junk * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);

		auto* item = storage::Factory<Item>::create(id, ctx.location_);
		p->list.push_back(item);
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(EqShop * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);

		auto* item = storage::Factory<Item>::create(id, ctx.location_);;
		p->list.push_back(item);
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(Treasure * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "HiddenItem"))
	{
		auto* item = storage::Factory<HiddenItem>::create();
		p->list.push_back(item);
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(HiddenItem * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "ItemId"))
	{
		int id = conv::extractId(obj_name);
		auto* item = storage::Factory<Item>::create(id, ctx.location_);
		p->item = item;
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_kv(HiddenItem * p, std::string_view key, std::string_view value)
{
	BEGIN_PARSE_FOR(HiddenItem)
		PARSE_TO(LandType)
		PARSE_TO(Depth)
	END_PARSE()
	
	return false;
}

bool Handler::on_kv(Item * p, std::string_view key, std::string_view value)
{
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

	return false;
}

bool Handler::on_new_obj(HoleList * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "HoleId"))
	{
		int id = conv::extractId(obj_name);
		auto* item = storage::Factory<Hole>::create(id);
		p->list.push_back( item );
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_kv(Hole * p, std::string_view key, std::string_view value)
{
	BEGIN_PARSE_FOR(Hole)
		PARSE_TO(Star1Id)
		PARSE_TO(Star2Id)
		PARSE_TO(TurnsToClose)
	END_PARSE()
	
	return false;
}

bool Handler::on_new_obj(Garrison * p, std::string_view obj_name)
{
	if (Starts_with(obj_name, "WarriorId"))
	{
		int id = conv::extractId(obj_name);
		auto* item = storage::Factory<Warrior>::create(id, ctx.location_);
		p->list.push_back(item);
		ctx.stack.push({ item });
		return true;
	}
	return false;
}

bool Handler::on_new_obj(Warrior * p, std::string_view obj_name)
{
	return on_new_obj((Warrior::Inherit*) p, obj_name);
}

bool Handler::on_kv(Warrior * p, std::string_view key, std::string_view value)
{
	return on_kv((Warrior::Inherit*) p, key, value);
}

} // namespace parser
