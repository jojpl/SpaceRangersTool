#include "analyzer.hpp"
#include "performance_tracker.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <cmath>
#include <sstream>
#include <set>
#include <string_view>
#include <type_traits>

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
	Entities::Planet* from;
	Entities::Planet* to;
	int distance = 0;
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
	std::string p_from_name       = cut_to<12>(pr.path.from->PlanetName);
	std::string p_to_name         = cut_to<12>(pr.path.to->PlanetName);
	int qty                       = pr.path.from->ShopGoods.packed[(int)bd_good];
	int sale                      = pr.path.from->ShopGoodsSale.packed[(int)bd_good];
	int buy                       = pr.path.from->ShopGoodsBuy.packed[(int)bd_good];

	const static std::string templ = "%-12s ==> %-12s %-7s qty: %-5d (%4d - %-4d) profit: %d";
	size_t buf_sz = snprintf(nullptr, 0, templ.data(),
		p_from_name.data(), p_to_name.data(),
		good_name.data(), qty, sale, buy, bd_profit
	);
	auto buf = std::make_unique<char[]>(buf_sz + 1);
	snprintf(buf.get(), buf_sz + 1, templ.data(),
		p_from_name.data(), p_to_name.data(),
		good_name.data(), qty, sale, buy, bd_profit
	);
	os << buf;
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

struct Filter
{
	bool operator()(){return false;}
};

void calc_profits(Entities::Global *data, std::vector<Profit>& vp)
{
	std::set<std::string> skip_star_list_name
	{ "Тортугац", "Нифигац" };

	std::set<std::string> skip_star_list_owners
	{ "Klings" };	
	
	std::set<std::string> skip_planet_list_owner
	{ "None", "Kling" };

	for (planet_iterator it1(data->StarList.list); !it1.end(); it1.next())
	{
		Entities::Star*   s1 = *it1.starlist_iter;
		Entities::Planet* p1 = *it1.planetlist_iter;

		if (skip_star_list_name.count(s1->StarName))
			continue;

		if (skip_star_list_owners.count(s1->Owners))
			continue;

		if (skip_planet_list_owner.count(p1->Owner))
			continue;

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

			 
			if (skip_star_list_name.count(s2->StarName))
				continue;

			if (skip_star_list_owners.count(s2->Owners))
				continue;

			if (skip_planet_list_owner.count(p2->Owner))
				continue;

			int distance = (int) std::hypot(std::abs(s1->X - s2->X), std::abs(s1->Y - s2->Y));

			if (distance > 30)
				continue;

			//bool stop = p1->PlanetName == "Оннд" && p2->PlanetName == "Элкада";

			Profit p;
			p.path.from = p1;
			p.path.to   = p2;
			p.path.distance = distance;

			//p.best_deal = { Entities::GoodsEnum{}, INT_MIN};
			for (size_t item = 0; item < ENUM_COUNT(Entities::GoodsEnum); item++)
			{
				int aviable_qty = p1->ShopGoods.packed[item];
				int sale = p1->ShopGoodsSale.packed[item]; // from

				int buy = p2->ShopGoodsBuy.packed[item]; // to
				int delta_profit = aviable_qty *(buy - sale);

				p.delta_buy_sale[item] = delta_profit;
				
				auto& [bd_good, bd_profit] = p.best_deal;
				if (bd_profit < delta_profit)
				{
					bd_profit = delta_profit;
					bd_good = (Entities::GoodsEnum)item;
				}
			}

			auto&[bd_good, bd_profit] = p.best_deal;
			if(bd_profit < 1000) continue;

			vp.push_back(std::move(p));
		}
	}

	std::sort(vp.rbegin(), vp.rend(), 
		[](Profit& pr1, Profit& pr2)
		{
			auto&[bd_good1, bd_profit1] = pr1.best_deal;
			auto&[bd_good2, bd_profit2] = pr2.best_deal;

			return bd_profit1 < bd_profit2;
		}
	);
	dump_top(vp, 5);

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
	std::vector<Profit> vp;
	calc_profits(data, vp);
	return;
	//std::cout << data->Player->IFullName << std::endl;
}
