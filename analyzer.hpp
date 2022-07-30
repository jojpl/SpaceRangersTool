#pragma once

#include "analyzer_entities.h"
#include "filters.hpp"
#include "sorters.hpp"

using namespace filters; // fix it!
using namespace sorters; // fix it!
class analyzer
{
public:
	analyzer(Entities::Global * data_)
		:data(data_)
	{}

	void calc_profits();
	void analyze_profit();

private:
	void calc_profits(filter_ptr filt);
	filter_ptr createPathFilter();
	filter_ptr createGoodsFilter();
	filter_ptr createFilter();
	sorter_ptr createSort();

	Entities::Global * data = nullptr;
	sorter_ptr sorter_;
};

