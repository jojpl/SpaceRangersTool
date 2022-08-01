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
	// from
	Star* s1   = nullptr;
	// to
	Star* s2   = nullptr;

	int distance = 0;
};

struct Profit
{
	GoodsEnum good   = {};
	int aviable_qty  = 0;
	int sale         = 0;
	int buy          = 0;
	int delta_profit = 0;
};

struct TradeInfo
{
	Path   path;
	Profit profit;
};

using TradeInfos = std::array<TradeInfo, ENUM_COUNT(GoodsEnum)>;

// for calc profit for ShipBases
struct FakePlanet : Planet
{
	FakePlanet(ShipBases* obj)
		: Planet(-1, obj->location), 
		obj_(obj)
	{	}
	ShipBases* obj_;
};

} //namespace analyzer