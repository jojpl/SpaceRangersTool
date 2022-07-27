#include "analyzer.hpp"
#include "performance_tracker.hpp"
#include "programargs.hpp"
#include "factory.hpp"

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

struct FilterByPathCommon : IFilter
{
	FilterByPathCommon()          = default;
	virtual ~FilterByPathCommon() = default;

	inline static const std::set<std::string>
		skip_star_list_name
	{ "Тортугац", "Нифигац" };

	inline static const
		std::set<std::string> skip_star_list_owners
	{ "Klings" };

	inline static const
		std::set<std::string> skip_planet_list_owner
	{ "None", "Kling" };

	bool operator()(Profit& pr) {
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

struct FilterCurStar : IFilter
{
	FilterCurStar(int id)
		: s_from_id(id)
	{
	}

	int s_from_id;

	virtual ~FilterCurStar() = default;
	bool operator()(Profit& pr) {
		auto* s1 = pr.path.s1;

		if (s1->Id != s_from_id) // faster than name cmp
			return false;
		return true;
	}
};

struct FilterCurPlanet : IFilter
{
	FilterCurPlanet(int id)
		: p_from_id(id)
	{
	}

	int p_from_id;

	virtual ~FilterCurPlanet() = default;
	bool operator()(Profit& pr) {
		auto* p1 = pr.path.p1;
		
		if (p1->Id != p_from_id) // faster than name cmp
			return false;
		return true;
	}
};

struct FilterByPath : IFilter
{
	FilterByPath( options::Options opt )
		: opt_(opt)
	{
	}

	virtual ~FilterByPath() = default;

	options::Options opt_;

	bool operator()(Profit& pr){
		auto s1 = pr.path.s1;
		auto s2 = pr.path.s2;
		auto p1 = pr.path.p1;
		auto p2 = pr.path.p2;

		if(pr.path.distance > opt_.max_dist.value())
			return false;

		if (opt_.star_from && s1->StarName != opt_.star_from.value())
			return false;

		if (opt_.star_to && s2->StarName != opt_.star_to.value())
			return false;

		if (opt_.planet_from && p1->PlanetName != opt_.planet_from.value())
			return false;

		if (opt_.planet_to && p2->PlanetName != opt_.planet_to.value())
			return false;

		return true;
	}
};

struct FilterByProfit : IFilter
{
	FilterByProfit( options::Options opt )
		: min_profit_(opt.min_profit.value())
	{	}

	virtual ~FilterByProfit() = default;

	int min_profit_;

	bool operator()(Profit& pr){
		if (pr.delta_profit < min_profit_)
			return false;

		return true;
	}
};

// some template magic
template <typename ... Args>
struct AND_opt : IFilter
{
	//AND_opt(std::shared_ptr<Args>&& ... args)
	//	: filters( std::forward<std::shared_ptr<Args>>(args)... )
	//{	}

	AND_opt(std::shared_ptr<Args> ... args)
		: filters(args...)
	{	}

	virtual ~AND_opt() = default;
	
	std::tuple<std::shared_ptr<Args> ...> filters;

	// aka template labmda c++20
	struct Help_Me
	{
		Profit& pr;

		template<typename ... Args>
		bool operator()(std::shared_ptr<Args>& ... args)
		{
			// unfold to call for each (arg1(pr) && ... && argN(pr)), stop if false
			bool res = ((*args)(pr) && ...);
			if (res)
				return true;

			return false;
		}
	};

	bool operator()(Profit& pr){
		return std::apply(Help_Me{pr}, filters); // unfold tuple to args ...
	}

};


struct NOT_opt : IFilter
{
	NOT_opt(std::shared_ptr<IFilter> f)
		: f_(f)
	{	}

	std::shared_ptr<IFilter> f_;
	bool operator()(Profit& pr) {
		return !(*f_)(pr); 
	}
};

void apply_filter(std::vector<Profit>& vp, std::shared_ptr<IFilter> callable)
{
	performance_tracker tr(__FUNCTION__);
	// some inverted filter logic for remove_if context
	auto pos = std::remove_if(
		vp.begin(), vp.end(),
		NOT_opt(callable)
	);

	vp.erase(pos, vp.end());
}

void analyzer::calc_profits(std::shared_ptr<IFilter> filt)
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

std::shared_ptr<IFilter> analyzer::createFilter()
{
	auto opt = options::get_opt();
	auto f1 = std::make_shared<FilterByPathCommon>();
	auto f2 = std::make_shared<FilterByProfit>( opt );
	auto f3 = std::make_shared<FilterByPath>( opt );
	auto common_f = std::shared_ptr<IFilter>(new AND_opt(f1, f2, f3));
	if (opt.star_from_use_current)
	{
		auto* s = find_curstar(data->Player);
		if(!s) throw std::logic_error("Player's curstar not set!");
		auto f = std::make_shared<FilterCurStar>(s->Id);
		common_f = std::shared_ptr<IFilter>(new AND_opt(common_f, f));
	}
	if (opt.planet_from_use_current)
	{
		auto* p = find_curplanet(data->Player);
		if (!p) throw std::logic_error("Player's curplanet not set!");
		auto f = std::make_shared<FilterCurPlanet>(p->Id);
		common_f = std::shared_ptr<IFilter>(new AND_opt(common_f, f));
	}
	return common_f;
}

struct DefaultSorter : ISort
{
	DefaultSorter() = default;
	virtual ~DefaultSorter() = default;

	bool operator()(Profit& pr1, Profit& pr2) const override
	{
		return pr1.delta_profit < pr2.delta_profit;
	}
};

struct ProfitSorter : ISort
{
	ProfitSorter() = default;
	virtual ~ProfitSorter() = default;

	bool operator()(Profit& pr1, Profit& pr2) const override
	{
		return pr1.delta_profit < pr2.delta_profit;
	}
};

struct ASC_Sort_Wrapper: ISort
{
	ASC_Sort_Wrapper(std::shared_ptr<ISort> obj_)
		: obj(obj_)
	{	}

	std::shared_ptr<ISort> obj;
	bool operator()(Profit& pr1, Profit& pr2) const override
	{
		return (*obj)(pr2, pr1); //revert
	}
};

struct DistanceSorter : ISort
{
	DistanceSorter() = default;
	virtual ~DistanceSorter() = default;

	bool operator()(Profit& pr1, Profit& pr2) const override
	{
		return pr1.path.distance < pr2.path.distance;
	}
};

struct AndSorter : ISort
{
	using Ptr = std::shared_ptr<ISort>;
	AndSorter(Ptr p1, Ptr p2)
		: p1_(p1), p2_(p2) 
	{	}
	virtual ~AndSorter() = default;
	
	Ptr p1_;
	Ptr p2_;

	bool operator()(Profit& pr1, Profit& pr2) const override
	{
		if (!p1_->operator()(pr1, pr2) && !p1_->operator()(pr2, pr1)) //eq
		{
			return p2_->operator()(pr1, pr2);
		}
		return p1_->operator()(pr1, pr2);
	}
};

enum class SortDirection
{
	unknown,

	DESC,
	ASC,
};

enum class SortField
{
	unknown,

	profit,
	distance,
};

void from_string(SortField& f, std::string_view sw)
{
	if (boost::iequals(sw, "profit"))
		f = SortField::profit;
	else if(boost::iequals(sw, "distance"))
		f = SortField::distance;
	else
		throw std::logic_error("can't convert "  __FUNCTION__);
}

void from_string(std::pair<SortField, SortDirection>& f, std::string_view sw)
{
	using split_range_it = boost::iterator_range<std::string_view::iterator>;
	std::vector<split_range_it> split_param;
	boost::split(split_param, sw, [](char ch) { return ch == ':'; });

	if (split_param.size() == 0)
		throw std::logic_error("can't convert "  __FUNCTION__);

	for (size_t i = 0; i < split_param.size(); i++)
	{
		auto rng = split_param[i];
		std::string param = { rng.begin(), rng.end() };
		boost::trim(param);
		
		if(i == 0) 
			from_string(f.first, param);
		else if (i == 1 )
		{
			if (boost::istarts_with(param, "ASC"))
				f.second = SortDirection::ASC;
			else
				f.second = SortDirection::DESC;
		}
	}
}

std::shared_ptr<ISort> analyzer::createSort()
{
	std::shared_ptr<ISort> common;

	auto opt = options::get_opt();
	std::string val = opt.sort_by.value();

	//using split_range_it = boost::iterator_range<std::string_view::iterator>;
	using tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;
	tokenizer csv_tok ( val );
	for (const auto &t : csv_tok)
	{
		auto p = std::pair<SortField, SortDirection>();
		from_string(p, t);
		std::shared_ptr<ISort> s;

		if (p.first == SortField::distance)
			s = std::make_shared<DistanceSorter>();
		else if (p.first == SortField::profit)
			s = std::make_shared<ProfitSorter>();
		else
			s = std::make_shared<DefaultSorter>();
			
		
		if(p.second == SortDirection::ASC)
			s = std::make_shared<ASC_Sort_Wrapper>(s);

		if(!common) common = s;
		else
			common = std::make_shared<AndSorter>(common, s);
	}

	//auto s1 = std::make_shared<DistanceSorter>();
	//auto s2 = std::make_shared<ProfitSorter>();
	//common = std::make_shared<AndSorter>(s1, s2);
	return common;
}

void analyzer::calc_profits()
{
	std::shared_ptr<IFilter> common_f = createFilter();
	sorter_ = createSort();
	calc_profits(common_f);
}

void analyzer::analyze_profit()
{
	performance_tracker tr(__FUNCTION__);
	calc_profits();
}
