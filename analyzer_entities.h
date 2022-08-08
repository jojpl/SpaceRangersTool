#pragma once
#include "Entities.h"

namespace analyzer
{

using namespace Entities;

struct Path
{
	Location from;
	Location to;

	int distance = 0;
};

struct Profit
{
	GoodsEnum good   = {};
	int qty          = 0;
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

} //namespace analyzer