#pragma once
#include "Entities.h"

namespace analyzer
{

using namespace Entities;

struct Path
{
	// from
	Planet* p1 = nullptr;
	// to
	Planet* p2 = nullptr;
	int distance         = 0;
	// for additional info
	// from
	Star* s1   = nullptr;
	// to
	Star* s2   = nullptr;
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

using Profits = std::array<Profit, ENUM_COUNT(GoodsEnum)>;

} //namespace analyzer