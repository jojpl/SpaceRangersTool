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

struct Coord
{
	double X;
	double Y;
};

struct Content
{
	Coord xy;
	std::string content;
};

struct Path
{
	// from
	Entities::Planet* p1;
	// to
	Entities::Planet* p2;
	int distance = 0;
	// for additional info
	// from
	Entities::Star* s1;
	// to
	Entities::Star* s2;
};

struct Profit
{
	Path path;

	std::array<int, ENUM_COUNT(Entities::GoodsEnum)> delta_buy_sale;
	std::pair<Entities::GoodsEnum, int> best_deal;
};

struct Vis
{
	Vis()
	{ //::SetConsoleOutputCP(1251); 
	}

	static constexpr int max_width = 75;//80; //X
	static constexpr int max_height = 30;//50; //Y

	void normlize(std::vector<Content>& c)
	{
		double maxX = 0;
		double maxY = 0;
		for (auto& item: c)
		{
			if(maxX < item.xy.X) maxX = item.xy.X;
			if(maxY < item.xy.Y) maxY = item.xy.Y;
		}

		for (auto& item : c)
		{
			if( item.xy.X > 0.0)
			{
				item.xy.X = (item.xy.X * (max_width-2)) / maxX;
			}
			if( item.xy.Y > 0.0)
			{
				 item.xy.Y = (item.xy.Y * (max_height-2)) / maxY;
			}
		}

	}

	constexpr static int myround(double d)
	{
		//(0.98 ... 1.98) ->1
		int di = (int) d;
		double lb = (di - 1) + 0.98;
		double rb = di + 0.98;
		if(d<lb)      return di - 1;
		else if(d>rb) return di + 1;
		else          return di;
		//std::fmod(2.89, &i);
	}

	bool find(std::vector<Content>& c, int x, int y, Content*& ref)
	{
		auto f = std::find_if(c.cbegin(), c.cend(), [x, y](const Content& ct)
			{
				if(myround(ct.xy.X)==x && myround(ct.xy.Y)==y)
					return true;
				return false;
			}
		);

		if(f==c.cend())
			return false;

		ref = &c[std::distance(c.cbegin(), f)];
		return true;
	}

	void fill(std::vector<Content> c)
	{
		normlize(c);

		using h_line = std::array<char, max_width>;
		using lines = std::array<h_line, max_height>;

		lines screen;

		for (size_t y = 0; y < max_height; y++)
			for (size_t x = 0; x < max_width; x++)
				screen[y][x] = '\1';

		//auto in_range = [&c](int x, int y)->bool {};

		std::ostream& os = std::cout;
		constexpr char nl = '\n';


		for (size_t y = 0; y < max_height; y++)
		{
			for (size_t x = 0; x < max_width; x++)
			{
				if(screen[y][x]!='\1')
					continue;

				Content* ref = 0;
				if (find(c, x, y, ref))
				{
					int sz = ref->content.size();
					int pos = 0;
					while (sz && (x + pos < max_width))
					{
						screen[y][x + pos] = ref->content[pos];
						sz--;
						pos++;
					}
					// os << ref->content 
				}
			}
		}

		for (size_t y = 0; y < max_height; y++)
			for (size_t x = 0; x < max_width; x++)
				if(screen[y][x] == '\1') screen[y][x] = ' ';

		for (size_t y = 0; y < max_height; y++, os << nl)
			for (size_t x = 0; x < max_width; x++)
			{
				os << screen[y][x];
			}
	}
};


void fillStarList(std::vector<Content>& vc, Entities::Global * data)
{
	for (auto* star : data->StarList.list)
	{
		Content c;
		c.xy.X = star->X;
		c.xy.Y = star->Y;
		c.content += "*";//star->StarName;
		c.content += star->StarName[0];
		vc.push_back(std::move(c));
	}
}

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
	auto&[bd_good, bd_profit]     = pr.best_deal;
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

void fill_profit(Profit& p,
	Entities::Star* s1,
	Entities::Star* s2,
	Entities::Planet* p1,
	Entities::Planet* p2)
{
	p.path.p1 = p1;
	p.path.p2 = p2;
	p.path.s1 = s1;
	p.path.s2 = s2;
	p.path.distance = (int)std::hypot(std::abs(s1->X - s2->X), std::abs(s1->Y - s2->Y));

	//p.best_deal = { Entities::GoodsEnum{}, INT_MIN};
	for (size_t item = 0; item < ENUM_COUNT(Entities::GoodsEnum); item++)
	{
		int aviable_qty = p1->ShopGoods.packed[item];
		int sale = p1->ShopGoodsSale.packed[item]; // from

		int buy = p2->ShopGoodsBuy.packed[item]; // to
		int delta_profit = aviable_qty * (buy - sale);

		p.delta_buy_sale[item] = delta_profit;

		auto&[bd_good, bd_profit] = p.best_deal;
		if (bd_profit < delta_profit)
		{
			bd_profit = delta_profit;
			bd_good = (Entities::GoodsEnum)item;
		}
	}
}

struct IFilter
{
	// true - accept, false - decline
	virtual bool operator()(Profit&) = 0;
	~IFilter() = default;
};

struct FilterByPath : IFilter
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

	virtual bool operator()(Profit& pr) override {
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

struct FilterByProfit : IFilter
{
	FilterByProfit()
	{	}

	virtual bool operator()(Profit& pr) override {
		
		auto&[bd_good, bd_profit] = pr.best_deal;
		if (bd_profit < 1000) 
			return false;

		return true;
	}
};

// some template magic
template <typename ... Args>
struct AND_opt : IFilter
{
	AND_opt(Args&& ... args)
		: filters( std::forward<Args>(args)... )
	{	}

	AND_opt(Args& ... args)
		: filters(args...)
	{	}
	
	std::tuple<Args ...> filters;

	//template <typename T, typename ... Args2>
	//bool call(Args& ... args)
	//{
	//	if ((args(pr) && ...))
	//		return true;

	//	return false;
	//}

	// aka template labmda c++20
	struct Help_Me
	{
		Profit& pr;

		template<typename ... Args>
		bool operator()(Args ... args)
		{
			// unfold to call for each (arg1(pr) && ... && argN(pr)), stop if false
			if ((args(pr) && ...))
				return true;

			return false;
		}
	};

	virtual bool operator()(Profit& pr) override {
		return std::apply(Help_Me{pr}, filters); // unfold tuple to args ...
	}
};

void analyzer::calc_profits()
{
	std::vector<Profit> vp;

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

			//if (s1 != s2) // inner Star
			//	continue;
			
			//if (s1->StarName != "Денебола") // inner Star
			//	continue;


		//	int distance = (int) std::hypot(std::abs(s1->X - s2->X), std::abs(s1->Y - s2->Y));

		//	if (distance > 30)
		//		continue;

			//bool stop = p1->PlanetName == "Оннд" && p2->PlanetName == "Элкада";

			Profit p;
			fill_profit(p, s1, s2, p1, p2);

			
		//	if(bd_good == Entities::GoodsEnum::Minerals) continue;
		//	if(bd_good == Entities::GoodsEnum::Narcotics) continue;
		//	if(bd_good == Entities::GoodsEnum::Food) continue;

			vp.push_back(std::move(p));
		}
	}

	auto f1 = FilterByProfit{};
	auto f2 = FilterByPath{};
	auto common_f = AND_opt( f1, f2);

	std::tuple<std::string, int> tup;
	auto fffff = std::get<0>(tup);
	auto fffff2 = std::get<1>(tup);

	//std::index_sequence_for;

	auto pos = std::remove_if(
			vp.begin(), vp.end(), 
			//common_f
			AND_opt(AND_opt(FilterByProfit{}, FilterByPath{}), FilterByPath{})
	);

	vp.erase(pos, vp.end());

	std::sort(vp.rbegin(), vp.rend(), 
		[](Profit& pr1, Profit& pr2)
		{
			auto&[bd_good1, bd_profit1] = pr1.best_deal;
			auto&[bd_good2, bd_profit2] = pr2.best_deal;

			return bd_profit1 < bd_profit2;
		}
	);

	dump_top(vp, 10);

	return;
}

void analyzer::draw_stars_ASCII_pic()
{
	//std::vector<Content> vc {
	//	{ {0.0, 0.0}, "****"},
	//	{ {0.0, 1.0}, "*****************"},
	//	{ {0.0, 2.0}, "**"},
	//};
	std::vector<Content> vc;
	fillStarList(vc, data);

	Vis v;
	v.fill(vc);
}

void analyzer::analyze_profit()
{
	performance_tracker tr;
	calc_profits();
	return;
	//std::cout << data->Player->IFullName << std::endl;
}
