#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

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

	int qty                       = ti.profit.aviable_qty;
	int sale                      = ti.profit.sale;
	int buy                       = ti.profit.buy;
	int purchase                  = qty*sale;
	
	// stars add info
	std::string s_from_name       = cut_to<15>(ti.path.s1->StarName);
	std::string s_to_name         = cut_to<15>(ti.path.s2->StarName);
	int distance                  = ti.path.distance;

	const std::string templ = 
	"%-15s => %-15s distance: %-2d\n"
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

void fill_profits(TradeInfos& profits,
	Star*   s1,
	Star*   s2,
	Planet* p1,
	Planet* p2)
{
	int distance = (int)std::hypot(std::abs(s1->X - s2->X), std::abs(s1->Y - s2->Y));

	for (size_t item = 0; item < profits.size(); item++)
	{
		auto& p = profits[item];

		auto bd_good = (GoodsEnum)item;
		int aviable_qty = p1->ShopGoods.packed[item];
		int sale = p1->ShopGoodsSale.packed[item]; // from

		int buy = p2->ShopGoodsBuy.packed[item]; // to
		int delta_profit = aviable_qty * (buy - sale);

		p.path.p1 = p1;
		p.path.p2 = p2;
		p.path.s1 = s1;
		p.path.s2 = s2;
		p.path.distance = distance;
		
		p.profit.good = bd_good;
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
		[&callable](TradeInfo& ti){
			return !(*callable)(ti);
		}
	);
}

void analyzer::calc_profits(filter_ptr filt, sorter_ptr sorter)
{
	std::vector<TradeInfo> vti; 
	vti.reserve(1'000'000); // 8 * planets_qty^2

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
				fill_profits(profits, s1, s2, p1, p2);

				bool doskip = std::none_of(begin(profits), end(profits), 
					filters::FilterByMinProfit(options::get_opt())
				);

				if(!doskip)
					std::move(begin(profits), end(profits), back_inserter(vti));
			}
		}
	}

	// optimization - filter cut >90% of vp values usually.
	//auto f = filters::FilterByMinProfit(options::get_opt());
	//auto v1 = boost::remove_erase_if(vti, 
	//	[&f](TradeInfo& pr1)	{
	//		return !f(pr1);
	//	}
	//);

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

filter_ptr analyzer::createGoodsFilter()
{
	auto opt = options::get_opt();

	std::set<GoodsEnum> sg;
	std::vector<std::string> gn;
	const auto& m = model::get_map<GoodsEnum>();
	for (const auto& [k, v]: m) // Food, ... , NUM
	{
		sg.insert(v);
		gn.push_back({k.data(), k.size()});
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
	auto f2 = filter_ptr(new filters::FilterByMinProfit( opt ));
	auto f3 = createPathFilter();
	auto f4 = createGoodsFilter();
	filter_ptr common_f (new filters::AND_opt(f1, f2, f3, f4));
	return common_f;
}

sorter_ptr analyzer::createSort()
{
	sorter_ptr common;
	auto opt = options::get_opt();
	options::SortOptions& sort_options = opt.sort_options;

	for( auto p : sort_options)
	{
		sorter_ptr s;
		if (p.first == options::SortField::distance)
			s = std::make_shared<sorters::DistanceSorter>();
		else if (p.first == options::SortField::profit)
			s = std::make_shared<sorters::MaxProfitSorter>();
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

}//namespace analyzer