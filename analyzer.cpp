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

using namespace std::string_literals;

namespace analyzer
{

class planet_iterator
{
	using PlanetList_Iter = decltype(PlanetList::list)::iterator;
	using StarList_Iter   = decltype(StarList::list)::iterator;
	
	using StarList_t = decltype(StarList::list);
	StarList_t& starlist;
public:
	planet_iterator(StarList_t& starlist_)
		: starlist(starlist_)
	{
		starlist_iter   = starlist.begin();
		planetlist_iter = (*starlist_iter)->PlanetList.list.begin();
	}

	StarList_Iter     starlist_iter   {};
	PlanetList_Iter   planetlist_iter {};

	bool end()
	{
		if (starlist_iter == starlist.end()) 
			return true;
		if(planetlist_iter == (*starlist_iter)->PlanetList.list.end())
			return true;

		return false;
	}

	void next()
	{
		planetlist_iter++;
		if (planetlist_iter == (*starlist_iter)->PlanetList.list.end())
		{
			starlist_iter++;
			if (starlist_iter != starlist.end())
				planetlist_iter = (*starlist_iter)->PlanetList.list.begin();
		}
	}
};

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

std::ostream& operator<<(std::ostream& os, Profit& pr)
{
	auto bd_good                  = pr.good;
	auto bd_profit                = pr.delta_profit;
	std::string_view good_name_sw = conv::to_string(bd_good);
	std::string good_name         = cut_to<9>(good_name_sw);
	std::string p_from_name       = cut_to<15>(pr.path.p1->PlanetName);
	std::string p_to_name         = cut_to<15>(pr.path.p2->PlanetName);

	int qty                       = pr.aviable_qty;
	int sale                      = pr.sale;
	int buy                       = pr.buy;
	int purchase                  = qty*sale;
	
	// stars add info
	std::string s_from_name       = cut_to<15>(pr.path.s1->StarName);
	std::string s_to_name         = cut_to<15>(pr.path.s2->StarName);
	int distance                  = pr.path.distance;

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

void dump_top(std::ostream& os, std::vector<Profit>& vp, options::Options opt)
{
	int top_size = opt.count.value();

	int cnt = 0;
	for (auto& pr: vp)
	{
		os << pr << std::endl;
		if(++cnt == top_size)
			break;
	}
	std::cout << "show: " << cnt << " from total: " << vp.size() << std::endl;
}

void fill_profits(Profits& profits,
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
		
		p.good = bd_good;
		p.aviable_qty = aviable_qty;
		p.buy = buy;
		p.sale = sale;
		p.delta_profit = delta_profit;
	}
}

void apply_filter(std::vector<Profit>& vp, filter_ptr callable)
{
	performance_tracker tr(__FUNCTION__);
	
	// some inverted filter logic for remove_if context
	boost::remove_erase_if(vp,
		[&callable](Profit& pr){
			return !(*callable)(pr);
		}
	);
}

void analyzer::calc_profits(filter_ptr filt, sorter_ptr sorter)
{
	std::vector<Profit> vp; 
	vp.reserve(1'000'000);
	
	{
		performance_tracker tr("iter");
		for (planet_iterator it1(data->StarList.list); !it1.end(); it1.next())
		{
			Star*   s1 = *it1.starlist_iter;
			Planet* p1 = *it1.planetlist_iter;

			for (planet_iterator it2(data->StarList.list); !it2.end(); it2.next())
			{
				Star*   s2 = *it2.starlist_iter;
				Planet* p2 = *it2.planetlist_iter;

				if (p1 == p2)
					continue;

				Profits profits;
				fill_profits(profits, s1, s2, p1, p2);
				std::move(profits.begin(), profits.end(),
					std::back_inserter(vp));
			}
		}
	}

	// optimization - filter cut >90% of vp values usually.
	//auto f = FilterByMinProfit(options::get_opt());
	//auto v1 = boost::remove_erase_if(vp, 
	//	[&f](Profit& pr1)	{
	//		return !f(pr1);
	//	}
	//);

	apply_filter(vp, filt);

	std::sort(vp.rbegin(), vp.rend(),
		[sorter](Profit& pr1, Profit& pr2) {
			return (*sorter)(pr1, pr2);
		}
	);

	dump_top(std::cout, vp, options::get_opt());

	return;
}

const Star*
find_star_by_name(std::string_view sw)
{
	return Factory<Star>::find(
		[sw](const Star& s) {
			return s.StarName == sw;
		}
	);
}

const Star*
find_star_by_id(int id)
{
	return Factory<Star>::find(
		[id](const Star& s) {
			return s.Id == id;
		}
	);
}

const Star*
find_curstar(Player* p)
{
	int id = p->ICurStarId;
	return find_star_by_id(id);
}

const Planet*
find_planet_by_name(std::string_view sw)
{
	return Factory<Planet>::find(
		[sw](const Planet& s) {
			return s.PlanetName == sw;
		}
	);
}

const Planet*
find_curplanet(Player* p)
{
	auto name = p->IPlanet;
	if(name.empty()) return nullptr;

	return find_planet_by_name(name);
}

filter_ptr analyzer::createPathFilter()
{
	auto opt = options::get_opt();
	int max_dist = opt.max_dist.value();
	auto* cur_s = find_curstar(data->Player);
	if (!cur_s) throw std::logic_error("Player's curstar not set (sic!)");
	auto* cur_p = find_curplanet(data->Player);
	
	// reslove id's using options
	int s1_id = 0, s2_id = 0,
		p1_id = 0, p2_id = 0;

	if (opt.star_from_use_current) 
		s1_id = cur_s->Id;
	else if (opt.star_from)
	{
		auto name = opt.star_from.value();
		auto* s = find_star_by_name(name);
		if (!s) throw std::logic_error(name + " star not found!");
		s1_id = s->Id;
	}

	if (opt.star_to_use_current)
		s2_id = cur_s->Id;
	else if (opt.star_to)
	{
		auto name = opt.star_to.value();
		auto* s = find_star_by_name(name);
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
		auto* p = find_planet_by_name(name);
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
		auto* p = find_planet_by_name(name);
		if (!p) throw std::logic_error(name + " planet not found!");
		p2_id = p->Id;
	}

	// create filter
	return filter_ptr(new filters::FilterByPath_v2(
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