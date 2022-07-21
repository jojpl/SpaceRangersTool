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

struct Profit
{
	Entities::Star* star;

	
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
	std::set<float> vx;
	std::set<float> vy;
	for (auto* star : data->StarList.list)
	{
		Content c;
		c.xy.X = star->X; vx.insert(star->X);
		c.xy.Y = star->Y; vy.insert(star->Y);
		c.content += "*";//star->StarName;
		c.content += star->StarName[0];
		vc.push_back(std::move(c));
	}
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


	Vis v;
	v.fill(vc);
	return;
	//std::cout << data->Player->IFullName << std::endl;
}
