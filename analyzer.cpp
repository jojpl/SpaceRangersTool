#include "analyzer.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <cmath>
#include <sstream>
#include <set>
#include <string_view>
#include <type_traits>

#include <windows.h>

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
	{ ::SetConsoleOutputCP(1251); }

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
	
	Entities::Global* data;

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
		if (starlist_iter != starlist.end())
		{
			auto local_end =(*starlist_iter)->PlanetList.list.end();
			return local_end == planetlist_iter;
		}
		return true;
	}

	void next()
	{
		if (starlist_iter != starlist.end())
		{
			if (planetlist_iter != (*starlist_iter)->PlanetList.list.end())
			{
				planetlist_iter++;
				if (planetlist_iter == (*starlist_iter)->PlanetList.list.end())
				{
					starlist_iter++;
					if (starlist_iter != starlist.end())
						planetlist_iter = (*starlist_iter)->PlanetList.list.begin();
				}
			}
		}
	}
};

struct Filter
{
	bool operator()(){return false;}
};

void calc_profits(Entities::Global *data)
{
	std::vector<Profit> vp;
	std::set<std::string> skip_star_list_name
	{ "Тортугац", "Нифигац" };

	std::set<std::string> skip_star_list_owners
	{ "Klings" };

	for (planet_iterator it1(data->StarList.list); !it1.end(); it1.next())
	{
		if (skip_star_list_name.count((*it1.starlist_iter)->StarName))
			continue;

		if (skip_star_list_owners.count((*it1.starlist_iter)->Owners))
			continue;

		for (planet_iterator it2(data->StarList.list); !it2.end(); it2.next())
		{
			Entities::Planet* p1 = *it1.planetlist_iter;
			Entities::Planet* p2 = *it2.planetlist_iter;
			Entities::Star*   s1 = *it1.starlist_iter;
			Entities::Star*   s2 = *it2.starlist_iter;

			int distance = (int) std::hypot(std::abs(s1->X - s2->X), std::abs(s1->Y - s2->Y));

			if (distance > 30)
				continue;

			if (*it1.planetlist_iter == *it2.planetlist_iter)
				continue;
			 
			if (skip_star_list_name.count(s2->StarName))
				continue;

			if (skip_star_list_owners.count(s2->Owners))
				continue;


			Profit p;
			p.path.from = p1;
			p.path.to   = p2;
			p.path.distance = distance;

			//p.best_deal = { Entities::GoodsEnum{}, INT_MIN};
			for (size_t item = 0; item < ENUM_COUNT(Entities::GoodsEnum); item++)
			{
				int qty = p1->ShopGoods.packed[item];
				int buy = p1->ShopGoodsBuy.packed[item];

				int sale = p2->ShopGoodsSale.packed[item];
				int delta_profit = qty*(sale - buy);

				p.delta_buy_sale[item] = delta_profit;
				
				auto& [bd_good, bd_profit] = p.best_deal;
				if (bd_profit < delta_profit)
				{
					bd_profit = delta_profit;
					bd_good = (Entities::GoodsEnum)item;
				}
			}

			vp.push_back(std::move(p));
		}
	}

	std::sort(vp.rbegin(), vp.rend(), 
		[](Profit& pr1, Profit& pr2)
		{
			//int max1 = 0;
			//for (auto val : pr1.delta_buy_sale)
			//{
			//	if(max1 < val) max1 = val;
			//}

			//int max2 = 0;
			//for (auto val : pr2.delta_buy_sale)
			//{
			//	if (max2 < val) max2 = val;
			//}

			//return max1 < max2;
			auto&[bd_good1, bd_profit1] = pr1.best_deal;
			auto&[bd_good2, bd_profit2] = pr2.best_deal;

			return bd_profit1 < bd_profit2;
		}
	);
	return;
}

void analyzer::analyze_some(Entities::Global * data)
{
	//std::vector<Content> vc {
	//	{ {0.0, 0.0}, "****"},
	//	{ {0.0, 1.0}, "*****************"},
	//	{ {0.0, 2.0}, "**"},
	//};
	std::vector<Content> vc;
	fillStarList(vc, data);

	calc_profits(data);

	Vis v;
	v.fill(vc);
	return;
	//std::cout << data->Player->IFullName << std::endl;
}
