#pragma once
#include "model.hpp"

struct Path
{
	// from
	Entities::Planet* p1 = nullptr;
	// to
	Entities::Planet* p2 = nullptr;
	int distance         = 0;
	// for additional info
	// from
	Entities::Star* s1   = nullptr;
	// to
	Entities::Star* s2   = nullptr;
};

struct Profit
{
	Path path;

	Entities::GoodsEnum good = {};
	int aviable_qty          = 0;
	int sale                 = 0;
	int buy                  = 0;
	int delta_profit         = 0;
};

class analyzer
{
	Entities::Global * data = nullptr;
public:
	analyzer(Entities::Global * data_)
		:data(data_)
	{}

	void calc_profits();

	void draw_stars_ASCII_pic();
	void analyze_profit();
};

