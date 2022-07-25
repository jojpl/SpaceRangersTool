#include "analyzer.hpp"
#include "performance_tracker.hpp"

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
#include <utility>

using Profits = std::array<Profit, ENUM_COUNT(Entities::GoodsEnum)>;

class planet_iterator
{
	using PlanetList_Iter = decltype(Entities::PlanetList::list)::iterator;
	using StarList_Iter   = decltype(Entities::StarList::list)::iterator;
	
	decltype(Entities::StarList::list)& starlist;
public:
	planet_iterator(decltype(Entities::StarList::list)& starlist_)
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
std::string cut_to(std::string s)
{
	//static const char ellipse[] = "...";
	if(s.size() <=Cnt) return s;
	//return s.substr(0, Cnt - 3) + ellipse;
	return s.substr(0, Cnt);
}

std::ostream& operator<<(std::ostream& os, Profit& pr)
{
	auto bd_good                  = pr.good;
	auto bd_profit                = pr.delta_profit;
	std::string_view good_name_sw = model::converter<Entities::GoodsEnum>::to_string(bd_good);
	std::string good_name         = cut_to<7>({good_name_sw.data(), good_name_sw.size()});
	std::string p_from_name       = cut_to<12>(pr.path.p1->PlanetName);
	std::string p_to_name         = cut_to<12>(pr.path.p2->PlanetName);

	int qty                       = pr.path.p1->ShopGoods.packed[(int)bd_good];
	int sale                      = pr.path.p1->ShopGoodsSale.packed[(int)bd_good];
	int buy                       = pr.path.p1->ShopGoodsBuy.packed[(int)bd_good];
	
	// stars add info
	std::string s_from_name       = cut_to<12>(pr.path.s1->StarName);
	std::string s_to_name         = cut_to<12>(pr.path.s2->StarName);

	const static std::string templ = 
	"%-12s ==> %-12s\n"
	"%-12s ==> %-12s %-7s qty: %-5d (%4d - %-4d) profit: %d";

	size_t buf_sz = snprintf(nullptr, 0, templ.data(),
		s_from_name.data(), s_to_name.data(),
		p_from_name.data(), p_to_name.data(),
		good_name.data(), qty, sale, buy, bd_profit
	);
	auto buf = std::make_unique<char[]>(buf_sz + 1);
	snprintf(buf.get(), buf_sz + 1, templ.data(),
		s_from_name.data(), s_to_name.data(),
		p_from_name.data(), p_to_name.data(),
		good_name.data(), qty, sale, buy, bd_profit
	);
	os << buf << std::endl;
	return os;
}

void dump_top(std::vector<Profit>& vp, int top_size)
{
	std::ostream& os = std::cout;
	int cnt = 0;
	for (auto& pr: vp)
	{
		os << pr << std::endl;
		if(++cnt == top_size)
			break;
	}
}


void fill_profits(Profits& profits,
	Entities::Star* s1,
	Entities::Star* s2,
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

struct IFilter
{
	// true - accept, false - decline
	virtual bool operator()(Profit&) = 0;
	~IFilter() = default;
};

struct FilterByPath
{
	FilterByPath()
	{	}

	inline static std::set<std::string> 
	skip_star_list_name
	{ "Тортугац", "Нифигац" };

	inline static
	std::set<std::string> skip_star_list_owners
	{ "Klings" };	
	
	inline static
	std::set<std::string> skip_planet_list_owner
	{ "None", "Kling" };

	bool operator()(Profit& pr){
		auto s1 = pr.path.s1;
		auto s2 = pr.path.s2;
		auto p1 = pr.path.p1;
		auto p2 = pr.path.p2;

		if (skip_star_list_name.count(s1->StarName))
			return false;
		if (skip_star_list_name.count(s2->StarName))
			return false;

		if (skip_star_list_owners.count(s1->Owners))
			return false;
		if (skip_star_list_owners.count(s2->Owners))
			return false;

		if (skip_planet_list_owner.count(p1->Owner))
			return false;
		if (skip_planet_list_owner.count(p2->Owner))
			return false;

		return true;
	}
};

struct FilterByProfit
{
	FilterByProfit()
	{	}

	bool operator()(Profit& pr){
		if (pr.delta_profit < 1000) 
			return false;

		return true;
	}
};

// some template magic
template <typename ... Args>
struct AND_opt
{
	AND_opt(Args&& ... args)
		: filters( std::forward<Args>(args)... )
	{	}

	AND_opt(Args& ... args)
		: filters(args...)
	{	}
	
	std::tuple<Args ...> filters;

	// aka template labmda c++20
	struct Help_Me
	{
		Profit& pr;

		template<typename ... Args>
		bool operator()(Args ... args)
		{
			// unfold to call for each (arg1(pr) && ... && argN(pr)), stop if false
			bool res = (args(pr) && ...);
			if (res)
				return true;

			return false;
		}
	};

	bool operator()(Profit& pr){
		return std::apply(Help_Me{pr}, filters); // unfold tuple to args ...
	}
};

template <typename Callable>
void apply_filter(std::vector<Profit>& vp, Callable&& c)
{
	// some inverted filter logic for remove_if context
	auto pos = std::remove_if(
		vp.begin(), vp.end(),
		std::not_fn(std::forward<Callable>(c))
	);

	vp.erase(pos, vp.end());
}

void analyzer::calc_profits()
{
	std::vector<Profit> vp; 
	vp.reserve(1'000'000);

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

	auto f1 = FilterByProfit{};
	auto f2 = FilterByPath{};
	auto common_f = AND_opt( f1, f2);

	apply_filter(vp, common_f);

	std::sort(vp.rbegin(), vp.rend(), 
		[](Profit& pr1, Profit& pr2)
		{
			return pr1.delta_profit < pr2.delta_profit;
		}
	);

	std::cout << "total is: " << vp.size() << std::endl;
	dump_top(vp, 10);

	return;
}

void analyzer::analyze_profit()
{
	performance_tracker tr;
	calc_profits();
}
