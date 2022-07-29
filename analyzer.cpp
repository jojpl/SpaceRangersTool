#include "analyzer.hpp"
#include "performance_tracker.hpp"
#include "programargs.hpp"
#include "factory.hpp"
#include "model.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <cmath>
#include <sstream>
#include <set>
#include <string_view>
#include <type_traits>
#include <tuple>
#include <optional>
#include <utility>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>

using Profits = std::array<Profit, ENUM_COUNT(Entities::GoodsEnum)>;
using namespace std::string_literals;

class planet_iterator
{
	using PlanetList_Iter = decltype(Entities::PlanetList::list)::iterator;
	using StarList_Iter   = decltype(Entities::StarList::list)::iterator;
	
	using StarList_t = decltype(Entities::StarList::list);
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

std::ostream& operator<<(std::ostream& os, Profit& pr)
{
	auto bd_good                  = pr.good;
	auto bd_profit                = pr.delta_profit;
	std::string_view good_name_sw = model::converter<Entities::GoodsEnum>::to_string(bd_good);
	std::string good_name         = cut_to<5>(good_name_sw);
	std::string p_from_name       = cut_to<15>(pr.path.p1->PlanetName);
	std::string p_to_name         = cut_to<15>(pr.path.p2->PlanetName);

	int qty                       = pr.path.p1->ShopGoods.packed[(int)bd_good];
	int sale                      = pr.path.p1->ShopGoodsSale.packed[(int)bd_good];
	int buy                       = pr.path.p1->ShopGoodsBuy.packed[(int)bd_good];
	
	// stars add info
	std::string s_from_name       = cut_to<15>(pr.path.s1->StarName);
	std::string s_to_name         = cut_to<15>(pr.path.s2->StarName);
	int distance                  = pr.path.distance;

	const static std::string templ = 
	"%-15s => %-15s distance: %3d\n"
	"%-15s -- %-15s %-5s q: %-5d (%4d - %-4d) profit: %d";

	size_t buf_sz = 
	snprintf(nullptr, 0, templ.data(),
		s_from_name.data(), s_to_name.data(), distance,
		p_from_name.data(), p_to_name.data(),
		good_name.data(), qty, sale, buy, bd_profit
	);
	auto buf = std::make_unique<char[]>(buf_sz + 1);
	snprintf(buf.get(), buf_sz + 1, templ.data(),
		s_from_name.data(), s_to_name.data(), distance,
		p_from_name.data(), p_to_name.data(),
		good_name.data(), qty, sale, buy, bd_profit
	);
	os << buf << std::endl;
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
	Entities::Star*   s1,
	Entities::Star*   s2,
	Entities::Planet* p1,
	Entities::Planet* p2)
{
	int distance = (int)std::hypot(std::abs(s1->X - s2->X), std::abs(s1->Y - s2->Y));

	for (size_t item = 0; item < profits.size(); item++)
	{
		auto& p = profits[item];

		auto bd_good = (Entities::GoodsEnum)item;
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
		p.sale = buy;
		p.delta_profit = delta_profit;
	}
}

void apply_filter(std::vector<Profit>& vp, filter_ptr callable)
{
	performance_tracker tr(__FUNCTION__);
	// some inverted filter logic for remove_if context

	auto pos = std::remove_if(
		vp.begin(), vp.end(),
		NOT_opt(callable)
	);

	vp.erase(pos, vp.end());
}

void analyzer::calc_profits(filter_ptr filt)
{
	std::vector<Profit> vp; 
	vp.reserve(1'000'000);
	
	{
		performance_tracker tr("iter");
		for (planet_iterator it1(data->StarList.list); !it1.end(); it1.next())
		{
			Entities::Star*   s1 = *it1.starlist_iter;
			Entities::Planet* p1 = *it1.planetlist_iter;

			for (planet_iterator it2(data->StarList.list); !it2.end(); it2.next())
			{
				Entities::Star*   s2 = *it2.starlist_iter;
				Entities::Planet* p2 = *it2.planetlist_iter;

				if (p1 == p2)
					continue;

				Profits profits;
				fill_profits(profits, s1, s2, p1, p2);

				std::move(profits.begin(), profits.end(),
					std::back_inserter(vp));
			}
		}
	}

	apply_filter(vp, filt);

	std::sort(vp.rbegin(), vp.rend(), 
		[this](Profit& pr1, Profit& pr2) {
			return (*sorter_)(pr1, pr2);
		}
	);

	//std::ostream& os = std::cout;
	dump_top(std::cout, vp, options::get_opt());

	return;
}

const Entities::Star*
find_curstar(Entities::Player* p)
{
	int id = p->ICurStarId;
	return Factory<Entities::Star>::find(
		[id](const Entities::Star& s) {
			return s.Id == id;
		}
	);
}

const Entities::Planet*
find_curplanet(Entities::Player* p)
{
	auto name = p->IPlanet;
	if(name.empty()) return nullptr;

	return Factory<Entities::Planet>::find(
		[name](const Entities::Planet& s) {
			return s.PlanetName == name;
		}
	);
}

filter_ptr analyzer::createFilter()
{
	auto opt = options::get_opt();
	auto f1 = std::make_shared<FilterByPathCommon>();
	auto f2 = std::make_shared<FilterByProfit>( opt );
	auto f3 = std::make_shared<FilterByPath>( opt );
	
	filter_ptr common_f (new AND_opt(f1, f2, f3));
	if (opt.star_from_use_current)
	{
		auto* s = find_curstar(data->Player);
		if(!s) throw std::logic_error("Player's curstar not set!");
		
		auto f = std::make_shared<FilterCurStar>(s->Id);
		common_f = filter_ptr(new AND_opt(common_f, f));
	}
	if (opt.planet_from_use_current)
	{
		auto* p = find_curplanet(data->Player);
		if (!p) throw std::logic_error("Player's curplanet not set!");
		
		auto f = std::make_shared<FilterCurPlanet>(p->Id);
		common_f = filter_ptr(new AND_opt(common_f, f));
	}
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
			s = std::make_shared<DistanceSorter>();
		else if (p.first == options::SortField::profit)
			s = std::make_shared<ProfitSorter>();
		else
			s = std::make_shared<DefaultSorter>();
			
		if(p.second == options::SortDirection::ASC)
			s = std::make_shared<ASC_Sort_Wrapper>(s);

		if(!common) 
			common = s;
		else
			common = std::make_shared<AndSorter>(common, s);
	}

	return common;
}

void analyzer::calc_profits()
{
	filter_ptr common_f = createFilter();
	sorter_ = createSort();
	calc_profits(common_f);
}

void analyzer::analyze_profit()
{
	performance_tracker tr(__FUNCTION__);
	calc_profits();
}
