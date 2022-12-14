#include "analyzer.hpp"
#include "performance_tracker.hpp"
#include "programargs.hpp"
#include "factory.hpp"
#include "model.hpp"
#include "convert.h"
#include "common_algo.h"
#include "datetime.h"
#include "con_text_coloring.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <cmath>
#include <sstream>
#include <set>
#include <optional>
#include <variant>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

using namespace std::string_literals;
using namespace std::placeholders;

namespace analyzer
{

struct star_names_list
{
	star_names_list()
	{
		const auto& stars = storage::get<Star>();
		std::transform(std::cbegin(stars), std::cend(stars), std::back_inserter(list),
			[](const Star& s){
				return std::string_view{s.StarName};
			}
		);
	}
	std::vector<std::string_view> list;
};

struct planet_names_list
{
	planet_names_list()
	{
		const auto& planets = storage::get<Planet>();
		std::transform(std::cbegin(planets), std::cend(planets), std::back_inserter(list),
			[](const Planet& p) {
				return std::string_view{ p.PlanetName };
			}
		);
	}
	std::vector<std::string_view> list;
};

Star* find_star_by_name_soft(std::string_view sw)
{
	static star_names_list names;
	auto pos = common_algo::soft_search(sw, names.list);
	if(pos == sw.npos) return nullptr;
	return storage::find_star_by_name(names.list[pos]);
}

Planet* find_planet_by_name_soft(std::string_view sw)
{
	static planet_names_list names;
	auto pos = common_algo::soft_search(sw, names.list);
	if (pos == sw.npos) return nullptr;
	return storage::find_planet_by_name(names.list[pos]);
}

int get_distance(const Star* s1, const Star* s2)
{
	return (int)std::hypot(s1->X - s2->X, s1->Y - s2->Y);
}

int get_distance(const Location& loc1, const Location& loc2)
{
	return (int)std::hypot(loc1.star->X - loc2.star->X, loc1.star->Y - loc2.star->Y);
}

template<size_t Cnt>
std::string cut_to(std::string_view s)
{
	return { s.data(), std::min(Cnt, s.size()) };
}

template<typename ... Args>
std::string string_format(const char* format, Args ... args)
{
	int buf_sz = snprintf(nullptr, 0, format, args...);
	auto buf = std::make_unique<char[]>(buf_sz + 1);
	snprintf(buf.get(), buf_sz + 1, format, args...);

	return std::string(buf.get());
}

std::ostream& operator<<(std::ostream& os, TradeInfo& ti)
{
	auto bd_good                  = ti.profit.good;
	auto bd_profit                = ti.profit.delta_profit;
	std::string_view good_name_sw = conv::to_string(bd_good);
	std::string good_name         = cut_to<9>(good_name_sw);
	std::string p_from_name       = ti.path.from.planet ?
									cut_to<15>(ti.path.from.planet->PlanetName)
								  : "#Ship-shop";
	std::string p_to_name         = ti.path.to.planet ?
									cut_to<15>(ti.path.to.planet->PlanetName)
								  : "#Ship-shop";

	int qty                       = ti.profit.aviable_qty; // make diff is opt set
	int sale                      = ti.profit.sale;
	int buy                       = ti.profit.buy;
	int purchase                  = qty*sale;
	
	// stars add info
	std::string s_from_name       = cut_to<15>(ti.path.from.star->StarName);
	std::string s_to_name         = cut_to<15>(ti.path.to.star->StarName);
	int distance                  = ti.path.distance;

	const std::string templ = 
	"%-15s => %-15s distance: %-3d\n"
	"%-15s -- %-15s %-9s p: %6d q: %-5d (%4d - %-4d) profit: %d";

	auto res = string_format(templ.data(),
		s_from_name.data(), s_to_name.data(), distance,
		p_from_name.data(), p_to_name.data(),
		good_name.data(), purchase, qty, sale, buy, bd_profit
	);
	
	os << res << std::endl;
	return os;
}

void dump_top(std::ostream& os, std::vector<TradeInfo>& vti, options::Options opt)
{
	int top_size = opt.count.value();

	int cnt = 0;
	for (auto& ti: vti)
	{
		os << ti << std::endl;
		if(++cnt == top_size)
			break;
	}
	std::cout << "show: " << cnt << " from total: " << vti.size() << std::endl;
}

void fill_tradeInfo(TradeInfos& profits,
	const ObjPrices& p1,
	const ObjPrices& p2,
	int aviable_storage)
{
	const auto& opt = options::get_opt();
	int distance = get_distance(p1.location, p2.location);

	for (size_t item = 0; item < profits.size(); item++)
	{
		auto& p = profits[item];

		auto bd_good = (GoodsEnum)item;
		int qty = p1.qty.packed[item];
		int aviable_qty = aviable_storage ?
			std::min(qty, aviable_storage) :
			qty;
		int sale = p1.sale.packed[item]; // from

		int buy = p2.buy.packed[item]; // to
		int delta_profit = aviable_qty * (buy - sale);

		p.path.from = p1.location;
		p.path.to = p2.location;

		p.path.distance = distance;

		p.profit.good = bd_good;
		p.profit.qty = qty;
		p.profit.aviable_qty = aviable_qty;
		p.profit.buy = buy;
		p.profit.sale = sale;
		p.profit.delta_profit = delta_profit;
	}
}

void apply_filter(std::vector<TradeInfo>& vti, filter_ptr callable)
{
	performance_tracker tr(__FUNCTION__);
	
	// some inverted filter logic for remove_if context
	boost::remove_erase_if(vti,
		[&callable](TradeInfo& ti) {
			return !(*callable)(ti);
		}
	);
}

int calc_aviable_players_storage(Player* player)
{
	int hull_size = 0;
	int cargo_volume = 0;
	for (const auto& item : player->EqList.list)
	{
		if(item->IType == Type::Hull)
		{
			hull_size = item->Size;
			continue;
		}
		cargo_volume += item->Size;
	}

	for (const auto& item : player->ArtsList.list) cargo_volume += item->Size;
	for (const auto& good_qty : player->Goods.packed) cargo_volume += good_qty;
	// TODO no art effects calc
	return hull_size - cargo_volume;
}

void analyzer::calc_all_trade_paths_info(std::vector<TradeInfo>& vti)
{
	performance_tracker tr("iter");
	vti.reserve(1'500'000); // 8 * ObjPrices_qty^2
	const auto& opt = options::get_opt();
	
	int aviable_storage =  opt.aviable_storage ? 
		opt.aviable_storage.value() : 0;

	aviable_storage = opt.aviable_storage_cur ? 
		calc_aviable_players_storage(data->Player) : aviable_storage;

	const auto& prices = storage::get<ObjPrices>();
	for (auto& p_from : prices)
	{
		for (auto& p_to : prices)
		{
			if (p_from.location.planet == p_to.location.planet)
				continue;

			TradeInfos profits;
			fill_tradeInfo(profits, p_from, p_to, aviable_storage);

			std::move(profits.begin(), profits.end(),
				std::back_inserter(vti));
		}
	}
}

void analyzer::calc_profits(filter_ptr filt, sorter_ptr sorter)
{
	std::vector<TradeInfo> vti; 
	calc_all_trade_paths_info(vti);

	const auto& opt = options::get_opt();
	// optimization - filter cut >90% of vp values usually.
	boost::remove_erase_if(vti,
		std::not_fn([min_profit_ = opt.min_profit](const TradeInfo& ti) {
			if (ti.profit.delta_profit < min_profit_)
				return false;

			return true;
		})
	);

	apply_filter(vti, filt);

	if (opt.tops)
	{
		//std::sort(vti.begin(), vti.end(),
		//	std::not_fn([this](const TradeInfo& ti1, const TradeInfo& ti2) {
		//		return (*tops_cmp_2)(ti1, ti2);
		//	})
		//);

		std::sort(vti.rbegin(), vti.rend(),
			[this](const TradeInfo& ti1, const TradeInfo& ti2) {
				return (*tops_cmp_2)(ti1, ti2);
			}
		);

		auto v1 = std::unique(vti.begin(), vti.end(),
			[this](const TradeInfo& ti1, const TradeInfo& ti2) {
				bool le = (*tops_cmp_1)(ti1, ti2);
				bool ge = (*tops_cmp_1)(ti2, ti1);
				bool eq = (!le && !ge); //eq
				return eq;
			}
		);

		vti.erase(v1, vti.end());

		std::sort(vti.rbegin(), vti.rend(),
			*sorters::make_sorter(&TradeInfo::profit, &Profit::delta_profit)
		);
	}

	std::sort(vti.rbegin(), vti.rend(),
		[sorter](const TradeInfo& ti1, const TradeInfo& ti2) {
			return (*sorter)(ti1, ti2);
		}
	);

	dump_top(std::cout, vti, options::get_opt());

	return;
}

void find_path_ids_from_opt(int& s1_id, int& s2_id,
	int& p1_id, int& p2_id)
{
	auto opt = options::get_opt();
	auto loc = storage::find_player_cur_location();
	auto* cur_s = loc.star;
	auto* cur_p = loc.planet;
	
	// reslove id's using options
	s1_id = 0, s2_id = 0,
		p1_id = 0, p2_id = 0;

	if (opt.star_from_use_current)
	{
		if (!cur_s) throw std::logic_error("Player's curstar not set (sic!)");
		s1_id = cur_s->Id;
	}
	else if (opt.star_from)
	{
		auto name = opt.star_from.value();
		//auto* s = storage::find_star_by_name(name);
		auto* s = find_star_by_name_soft(name);
		if (!s) throw std::logic_error(name + " star not found!");
		s1_id = s->Id;
	}

	if (opt.star_to_use_current)
	{
		if (!cur_s) throw std::logic_error("Player's curstar not set (sic!)");
		s2_id = cur_s->Id;
	}
	else if (opt.star_to)
	{
		auto name = opt.star_to.value();
		//auto* s = storage::find_star_by_name(name);
		auto* s = find_star_by_name_soft(name);
		if (!s) throw std::logic_error(name + " star not found!");
		s2_id = s->Id;
	}

	if (opt.planet_from_use_current)
	{
		if (!cur_p) throw std::logic_error("Player's curplanet not set");
		p1_id = cur_p->Id;
	}
	else if (opt.planet_from)
	{
		auto name = opt.planet_from.value();
		auto* p = find_planet_by_name_soft(name);
		if (!p) throw std::logic_error(name + " planet not found!");
		p1_id = p->Id;
	}

	if (opt.planet_to_use_current)
	{
		if (!cur_p) throw std::logic_error("Player's curplanet not set");
		p2_id = cur_p->Id;
	}
	else if (opt.planet_to)
	{
		auto name = opt.planet_to.value();
		auto* p = find_planet_by_name_soft(name);
		if (!p) throw std::logic_error(name + " planet not found!");
		p2_id = p->Id;
	}
}

filter_ptr analyzer::createPathFilter()
{
	int s1_id = 0, s2_id = 0, p1_id = 0, p2_id = 0;
	find_path_ids_from_opt(s1_id, s2_id, p1_id, p2_id);

	auto opt = options::get_opt();
	int max_dist = opt.max_dist.value();
	// create filter
	return filter_ptr(new filters::FilterByPath(
		max_dist,
		s1_id, s2_id,
		p1_id, p2_id)
		);
}

filter_ptr analyzer::createRadiusFilter()
{
	const auto& opt = options::get_opt();
	if(opt.search_radius){
		int radius = opt.search_radius.value();
		Star* curstar = storage::find_player_cur_star();
		const auto& stars = storage::get<Star>();

		std::vector<int> vi;
		for (const auto& star: stars)
		{
			if((int) get_distance(curstar, &star) <= radius)
				vi.push_back(star.Id);
		}
		
		return filter_ptr(new filters::FilterByStarFromIdArr(vi));
	}

	return filter_ptr(new filters::Nul_Opt());
}

filter_ptr analyzer::createGoodsFilter()
{
	auto opt = options::get_opt();

	const auto& gn = model::enums::get_strings<GoodsEnum>();
	const auto& ge = model::enums::get_enums<GoodsEnum>();
	std::set<GoodsEnum> sg{ ge.begin(), ge.end() };

	if(!opt.goods.empty()) 
	{
		sg.clear();
		for (const auto& gs_raw : opt.goods)
		{
			auto pos = common_algo::soft_search(gs_raw, gn);
			if(pos == gs_raw.npos)
				throw std::logic_error("Good named as \""s + gs_raw + "\" not set");
			auto gs = gn[pos];

			GoodsEnum g;
			conv::from_string(g, gs);
			sg.insert(g);
		}
	}

	for (const auto& gs_raw : opt.no_goods)
	{
		auto pos = common_algo::soft_search(gs_raw, gn);
		if (pos == gs_raw.npos)
			throw std::logic_error("Good named as \""s + gs_raw + "\" not set");
		auto gs = gn[pos];

		GoodsEnum g;
		conv::from_string(g, gs);
		sg.erase(g);
	}

	return filter_ptr(new filters::FilterGoods(sg));
}

filter_ptr analyzer::createFilter()
{
	auto opt = options::get_opt();
	auto f1 = filter_ptr(new filters::FilterByPathCommon());
	auto f2 = filter_ptr(new filters::FilterByMinProfit( opt.min_profit ));
	auto f3 = createPathFilter();
	auto f4 = createGoodsFilter();
	auto f5 = createRadiusFilter();
	filter_ptr common_f (new filters::AND_opt{f1, f2, f3, f4, f5});
	return common_f;
}

sorter_ptr createSortfromOpt(const options::SortOptions& sort_options)
{
	sorter_ptr common;

	for( auto [field, direction] : sort_options)
	{
		sorter_ptr s;
		switch (field)
		{
			case options::SortField::distance:
				s = sorters::make_sorter(&TradeInfo::path, &Path::distance);
				break;

			case options::SortField::profit:
				s = sorters::make_sorter(&TradeInfo::profit, &Profit::delta_profit);
				break;

			case options::SortField::star:
				s = sorters::make_sorter(&TradeInfo::path, &Path::from, &Location::star); //by raw pointer
				break;

			case options::SortField::planet:
				s = sorters::make_sorter(&TradeInfo::path, &Path::from, &Location::planet); //by raw pointer
				break;

			case options::SortField::good:
				s = sorters::make_sorter(&TradeInfo::profit, &Profit::good); //by raw pointer
				break;

			default:
				s = sorters::make_sorter(&TradeInfo::profit, &Profit::delta_profit);
				break;
		}

		if(direction == options::SortDirection::ASC)
			s = sorter_ptr(new sorters::ASC_Wrapper(s));

		if(!common)
			common = s;
		else
			common = std::make_shared<sorters::AndSorter>(common, s);
	}

	return common;
}

// mb move to ::sorters
sorter_ptr analyzer::createSort()
{
	const auto& opt = options::get_opt();
	if (opt.tops)
	{
		//"pr, st, pl, g, pr"
		//g, st, pl, pr
		options::SortOptions sort_options;
		sort_options.push_back({ options::SortField::good,   {} });
		sort_options.push_back({ options::SortField::star,   {} });
		sort_options.push_back({ options::SortField::planet, {} });
		tops_cmp_1 = createSortfromOpt(sort_options);
		sort_options.push_back({ options::SortField::profit, {} });
		tops_cmp_2 = createSortfromOpt(sort_options);
	}

	const options::SortOptions& sort_options = opt.sort_options;
	return createSortfromOpt(sort_options);
}

void analyzer::calc_profits()
{
	filter_ptr common_f = createFilter();
	sorter_ptr sorter = createSort();
	calc_profits(common_f, sorter);
}

void analyzer::analyze_profit()
{
	performance_tracker tr(__FUNCTION__);
	calc_profits();
}

std::ostream& dump_HiddenItem_info(std::ostream& os, const HiddenItem* hitem)
{
	auto* cur_s = storage::find_player_cur_star();
	if (!cur_s) throw std::logic_error("Find player err!");
	auto* item = hitem->item;
	int lt = hitem->LandType;
	std::string_view LandType_sw = hitem->LandType == 0 ? "Water" :
							hitem->LandType == 1 ? "Land" :
							hitem->LandType == 2 ? "Mount" :
							"???";
	std::string LandType{ LandType_sw };
	auto IType_sw =  conv::to_string(item->IType);
	std::string IType { IType_sw };
	auto Owner_sw =  conv::to_string(item->Owner);
	std::string Owner{ Owner_sw };

	const std::string templ =
		"%d,%s,%s,%s,%s,%d,%s,%s,%d";
	auto res = string_format(templ.data()
		, item->Id
		, item->IName.data()
		, IType.data()
		, item->location.star->StarName.data()
		, item->location.planet->PlanetName.data()
		, get_distance(cur_s, item->location.star)
		, Owner.data()
		, LandType.data()
		, hitem->Depth
	);

	colors::PrintColored(res);
	return os;
}

void analyzer::dump_treasures()
{
	const auto& items = storage::get<HiddenItem>();
	//auto& opt = options::get_opt();
	//if(opt)
	int s1_id = 0, s2_id = 0, p1_id = 0, p2_id = 0;
	find_path_ids_from_opt(s1_id, s2_id, p1_id, p2_id);

	std::ostream& os =  std::cout;
	auto head = "id,name,type,star,planet,dist,star_owners,landType,depth";
	os << head << "\n";
	for (const auto& item : items)
	{
		if (s1_id 
		&& item.item->location.star->Id != s1_id)
			continue;

		if (p1_id 
		&& item.item->location.planet 
		&& item.item->location.planet->Id != p1_id)
			continue;

		if(item.item->IType!=Type::Nod) continue;

		dump_HiddenItem_info(os, &item);
		std::cout << '\n';
	}
}

void analyzer::show_price()
{
	auto& opt = options::get_opt();
	if(opt.goods.empty())
		throw std::logic_error("Good for find not set!");
	if (opt.goods.size()>1)
		throw std::logic_error("Too many goods for find!");

	std::string good_name_raw = opt.goods[0];

	//goods name list
	auto gn = model::enums::get_strings<GoodsEnum>();

	auto pos = common_algo::soft_search(good_name_raw, gn);
	if (pos == good_name_raw.npos)
		throw std::logic_error("Good named as \""s + good_name_raw + "\" not set");
	auto good_name = gn[pos];

	GoodsEnum g;
	conv::from_string(g, good_name);

	struct Price
	{
		Location location;
		int distance_to_player = 0;
		//GoodsEnum good;
		int sale = 0;
		int buy = 0;
		int qty = 0;
	};

	std::vector<Price> vp;

	auto* cur_s = storage::find_player_cur_star();
	const auto& planets = storage::get<Planet>();
	for (const auto& p_from : planets)
	{
		const Star*   s1 = p_from.location.star;
		const Planet* p1 = &p_from;

		if(s1->Owners == OwnersGroup::Klings)
			continue;
		if(p1 && ( p1->Owner == Owner::Kling 
				|| p1->Owner == Owner::None ))
			continue;

		Price p;
		p.distance_to_player = get_distance(s1, cur_s);
		p.location = p_from.location;

		//p.good = g;
		p.buy = p1->ShopGoodsBuy.packed[(int)g];
		p.sale = p1->ShopGoodsSale.packed[(int)g];
		p.qty = p1->ShopGoods.packed[(int)g];

		vp.push_back(p);
	}

	const int max_dist = opt.max_dist.value();
	boost::remove_erase_if(vp, 
		[max_dist](const Price& pr1){
			 return pr1.distance_to_player > max_dist;
		}	
	);

	std::sort(vp.begin(), vp.end(),
		sorters::ASC_Wrapper(sorters::make_sorter(&Price::buy))
	);
	
	const std::string templ =
		"%-15s / %-15s distance: %-3d %-9s q: %-5d %4d/%-4d";

	int cnt = opt.count.value();
	for (const Price& pr : vp)
	{
		if(!cnt--) break;

		auto sn = cut_to<15>(pr.location.star->StarName);
		auto pn = cut_to<15>(pr.location.planet->PlanetName);

		auto res = string_format(templ.data(),
			sn.data(), pn.data(), pr.distance_to_player,
			good_name.data(), pr.qty, pr.sale, pr.buy
		);

		std::cout << res << std::endl;
	}
}

void analyzer::dump_holelist()
{
	const auto& holes = storage::get<Hole>();
	for (const auto& h : holes)
	{
		const std::string templ =
			"%-15s <==> %-15s TurnsToClose: %-3d";

		auto s_from = cut_to<15>(h.from.star->StarName);
		auto s_to   = cut_to<15>(h.to.star->StarName);

		auto res = string_format(templ.data(),
			s_from.data(), s_to.data(), h.TurnsToClose
		);

		std::cout << res << std::endl;
	}
}

void analyzer::print_game_date()
{
	auto str = datetime::get_cur_game_date_str(data->IDay);
	std::cout << str <<std::endl;
}

void analyzer::show_ritches()
{
	const auto& sh = storage::get<Ship>();
	struct Info 
	{
		string Name;
		string StarName;
		int Money;
	};

	vector<Info> vi;
	for (const auto& s: sh)
	{
		if (s.IType != Type::Transport)
			continue;

		vi.push_back({s.Name, s.location.star->StarName, s.Money});
	}

	std::sort(begin(vi), end(vi), 
		*sorters::make_sorter(&Info::Name)
	);

	for (const auto& i : vi)
	{
		const std::string templ =
			"%-15s %-20s %d";

		auto n = cut_to<15>(i.Name);
		auto sn = cut_to<15>(i.StarName);
		
		auto res = string_format(templ.data(),
			sn.data(), n.data(), i.Money
		);

		std::cout << res << std::endl;
	}
}

struct ItemReal
{
	std::optional<Type> IType;
	std::optional<std::string> IName;
	std::optional<int> Size;
	std::optional<int> MinDamage;
	std::optional<int> TechLevel;
};

bool is_item_accept(const ItemReal& search_item, Item* item)
{
	if (search_item.IType && item->IType != search_item.IType.value())
		return false;

	if (search_item.IName &&
		item->IName.find(search_item.IName.value()) == std::string::npos)
		return false;

	if (search_item.Size && item->Size < search_item.Size.value())
		return false;

	if (search_item.MinDamage && item->MinDamage
		&& item->MinDamage.value() < search_item.MinDamage.value())
		return false;

	if (search_item.TechLevel && item->TechLevel < search_item.TechLevel.value())
		return false;

	return true;
}

void analyzer::show_items_search_reminder()
{
	const auto& opt = options::get_opt();
	const auto& optItem = opt.itemSearch.value();
	
	ItemReal search_item;
	if (optItem.IType) 
		conv::from_string(search_item.IType,     optItem.IType.value());
	if (optItem.IName)
		conv::from_string(search_item.IName,     optItem.IName.value());
	if (optItem.Size)
		conv::from_string(search_item.Size,      optItem.Size.value());
	if (optItem.MinDamage)
		conv::from_string(search_item.MinDamage, optItem.MinDamage.value());
	if (optItem.TechLevel)
		conv::from_string(search_item.TechLevel, optItem.TechLevel.value());

	vector<std::pair<const Item*, std::variant<const Planet*, const Station*> >> vi;

	const auto& stations = storage::get<Station>();
	for (const auto& station : stations)
	{
		for (const auto& item : station.EqShop.list)
		{
			if(!is_item_accept(search_item, item))
				continue;
			vi.emplace_back(item, &station);
		}
	}
	const auto& planets = storage::get<Planet>();
	for (const auto& planet : planets)
	{
		if(planet.Owner == Owner::None 
		|| planet.Owner == Owner::Kling)
			continue;

		for (const auto& item : planet.EqShop.list)
		{
			if (!is_item_accept(search_item, item))
				continue;
			vi.emplace_back(item, &planet);
		}
	}

	std::ostream& os = std::cout;
	if(!vi.empty())
		colors::PrintColored("<color=255,0,255>Finded Items</color>\n");
	
	auto get_loc_name = [](const std::variant<const Planet*, const Station*>& loc)
	{
		auto* pl = std::get_if<const Planet*>(&loc);
		if (pl) return (*pl)->PlanetName.data();

		auto* st = std::get_if<const Station*>(&loc);
		if (st) return (*st)->IFullName.data();
		
		return "";
	};

	const std::string templ =
		"What: id: %d, name: %s cost %6d Where: %s %s";
	for (const auto& [item, loc] : vi)
	{
		auto res = string_format(templ.data()
			, item->Id
			, item->IName.data()
			, item->Cost
			, item->location.star->StarName.data()
			, get_loc_name(loc)
		);

		colors::PrintColored(res);
		os << std::endl;
	}
}

}//namespace analyzer