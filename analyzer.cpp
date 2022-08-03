#include "analyzer.hpp"
#include "performance_tracker.hpp"
#include "programargs.hpp"
#include "factory.hpp"
#include "model.hpp"
#include "convert.h"
#include "common_algo.h"

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
#include <tuple>
#include <optional>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/tokenizer.hpp>
#include <boost/move/iterator.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

using namespace std::string_literals;

namespace analyzer
{

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
	std::string p_from_name       = cut_to<15>(ti.path.p1->PlanetName);
	std::string p_to_name         = cut_to<15>(ti.path.p2->PlanetName);

	int qty                       = ti.profit.aviable_qty; // make diff is opt set
	int sale                      = ti.profit.sale;
	int buy                       = ti.profit.buy;
	int purchase                  = qty*sale;
	
	// stars add info
	std::string s_from_name       = cut_to<15>(ti.path.s1->StarName);
	std::string s_to_name         = cut_to<15>(ti.path.s2->StarName);
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

int get_distance(const Star* s1, const Star* s2)
{
	return (int)std::hypot(s1->X - s2->X, s1->Y - s2->Y);
}

void fill_tradeInfo(TradeInfos& profits,
	Star*   s1,
	Star*   s2,
	Planet* p1,
	Planet* p2)
{
	auto& opt = options::get_opt();
	int distance = get_distance(s1, s2);

	for (size_t item = 0; item < profits.size(); item++)
	{
		auto& p = profits[item];

		auto bd_good = (GoodsEnum)item;
		int qty = p1->ShopGoods.packed[item];
		int aviable_qty = opt.aviable_storage ?	
			std::min(qty, opt.aviable_storage.value()) :
			qty;
		int sale = p1->ShopGoodsSale.packed[item]; // from

		int buy = p2->ShopGoodsBuy.packed[item]; // to
		int delta_profit = aviable_qty * (buy - sale);

		p.path.p1 = p1;
		p.path.p2 = p2;
		p.path.s1 = s1;
		p.path.s2 = s2;
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

void analyzer::calc_profits(filter_ptr filt, sorter_ptr sorter)
{
	std::vector<TradeInfo> vti; 
	vti.reserve(1'000'000); // 8 * planets_qty^2
	const auto& opt = options::get_opt();
	{
		performance_tracker tr("iter");

		auto& planets = storage::get<Planet>();
		for (auto& p_from : planets)
		{
			Star*   s1 = p_from.location.star;
			Planet* p1 = &p_from;

			for (auto& p_to : planets)
			{
				Star*   s2 = p_to.location.star;
				Planet* p2 = &p_to;

				if (p1 == p2)
					continue;

				TradeInfos profits;
				fill_tradeInfo(profits, s1, s2, p1, p2);


				//std::copy_if(begin(profits), end(profits), back_inserter(vti),
				//	filters::FilterByMinProfit(opt)
				//	);
				std::move(profits.begin(), profits.end(),
					std::back_inserter(vti));
			}
		}
	}

	// optimization - filter cut >90% of vp values usually.
	auto f = filters::FilterByMinProfit(opt.min_profit);
	auto v1 = boost::remove_erase_if(vti, 
		[&f](const TradeInfo& tri) {
			return !f(tri.profit);
		}
	);

	apply_filter(vti, filt);

	std::sort(vti.rbegin(), vti.rend(),
		[sorter](const TradeInfo& ti1, const TradeInfo& ti2) {
			return (*sorter)(ti1, ti2);
		}
	);

	dump_top(std::cout, vti, options::get_opt());

	return;
}

filter_ptr analyzer::createPathFilter()
{
	auto opt = options::get_opt();
	int max_dist = opt.max_dist.value();
	auto* cur_s = data->Player->location.star;
	auto* cur_p = data->Player->location.planet;
	
	// reslove id's using options
	int s1_id = 0, s2_id = 0,
		p1_id = 0, p2_id = 0;

	if (opt.star_from_use_current)
	{
		if (!cur_s) throw std::logic_error("Player's curstar not set (sic!)");
		s1_id = cur_s->Id;
	}
	else if (opt.star_from)
	{
		auto name = opt.star_from.value();
		auto* s = storage::find_star_by_name(name);
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
		auto* s = storage::find_star_by_name(name);
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
		auto* p = storage::find_planet_by_name(name);
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
		auto* p = storage::find_planet_by_name(name);
		if (!p) throw std::logic_error(name + " planet not found!");
		p2_id = p->Id;
	}

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
		Star* curstar = data->Player->location.star;
		auto& stars = storage::get<Star>();

		std::vector<int> vi;
		for (auto& star: stars)
		{
			if((int) get_distance(curstar, &star) <= radius)
				vi.push_back(star.Id);
		}
		
		return filter_ptr(new filters::FilterByRadius(vi));
	}

	return filter_ptr(new filters::Nul_Opt());
}

filter_ptr analyzer::createGoodsFilter()
{
	auto opt = options::get_opt();

	std::set<GoodsEnum> sg;
	std::vector<std::string> gn = model::enums::get_strings<GoodsEnum>();
	for (const auto& e: model::enums::get_enums<GoodsEnum>()) // Food, ... , NUM
	{
		sg.insert(e);
	}

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
	filter_ptr common_f (new filters::AND_opt(f1, f2, f3, f4, f5));
	return common_f;
}

// mb move to ::sorters
sorter_ptr analyzer::createSort()
{
	sorter_ptr common;
	auto opt = options::get_opt();
	options::SortOptions& sort_options = opt.sort_options;

	for( auto p : sort_options)
	{
		sorter_ptr s;
		if (p.first == options::SortField::distance)
			//s = std::make_shared<sorters::DistanceSorter>();
			s = sorter_ptr(new sorters::CommonSorter2(&TradeInfo::path, &Path::distance));
		else if (p.first == options::SortField::profit)
			//s = std::make_shared<sorters::MaxProfitSorter>();
			s = sorter_ptr(new sorters::CommonSorter2(&TradeInfo::profit, &Profit::delta_profit));
		else if (p.first == options::SortField::star)
			s = sorter_ptr(new sorters::CommonSorter2(&TradeInfo::path, &Path::s1)); //by raw pointer
		else if (p.first == options::SortField::planet)
			s = sorter_ptr(new sorters::CommonSorter2(&TradeInfo::path, &Path::p1)); //by raw pointer
		else if (p.first == options::SortField::good)
			s = sorter_ptr(new sorters::CommonSorter2(&TradeInfo::profit, &Profit::good)); //by raw pointer
		else
			s = std::make_shared<sorters::DefaultSorter>();
			
		if(p.second == options::SortDirection::ASC)
			s = std::make_shared<sorters::ASC_Wrapper>(s);

		if(!common) 
			common = s;
		else
			common = std::make_shared<sorters::AndSorter>(common, s);
	}

	return common;
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

std::ostream& dump_Item_info(std::ostream& os, Item* item)
{
	// TODO - meditate about game model objects getter
	auto* cur_s = storage::find_player_cur_star();
	if(!cur_s) throw std::logic_error("Find player err!");

	auto res = fmt::format("{id},{name},{type},{star},{planet},{dist},{star_owners}"
		, fmt::arg("id", item->Id)
		, fmt::arg("name", item->IName)
		, fmt::arg("type", conv::to_string(item->IType))
		, fmt::arg("star", item->location.star->StarName)
		, fmt::arg("planet", item->location.planet->PlanetName)
		, fmt::arg("dist", get_distance(cur_s, item->location.star))
		, fmt::arg("star_owners", item->Owner )
		);

	return os << res;
}

std::ostream& dump_HiddenItem_info(std::ostream& os, HiddenItem* hitem)
{
	dump_Item_info(os, hitem->item);
	auto res = fmt::format(
	"{landType},{depth}", 
		fmt::arg("landType", hitem->LandType),
		fmt::arg("depth", hitem->Depth)
	);
	return os << res;
}

void analyzer::dump_treasures()
{
	auto& items = storage::get<HiddenItem>();
	//std::vector<HiddenItem_info> vhi;
	std::ostream& os =  std::cout;
	for (auto& item : items)
	{
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
	std::vector<std::string> gn = model::enums::get_strings<GoodsEnum>();

	auto pos = common_algo::soft_search(good_name_raw, gn);
	if (pos == good_name_raw.npos)
		throw std::logic_error("Good named as \""s + good_name_raw + "\" not set");
	auto good_name = gn[pos];

	GoodsEnum g;
	conv::from_string(g, good_name);

	struct Price
	{
		Entities::Location location;
		int distance_to_player;
		//GoodsEnum good;
		int sale;
		int buy;
		int qty;
	};

	std::vector<Price> vp;

	auto* cur_s = storage::find_player_cur_star();
	auto& planets = storage::get<Planet>();
	for (auto& p_from : planets)
	{
		Star*   s1 = p_from.location.star;
		Planet* p1 = &p_from;

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

	std::sort(vp.rbegin(), vp.rend(),
		sorters::CommonSorter1(&Price::buy)
	//	//[](const Price& pr1, const Price& pr2) {
	//	//	return pr1.buy < pr2.buy;
	//	//}
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

}//namespace analyzer