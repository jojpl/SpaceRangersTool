#pragma once

#include "analyzer_entities.h"
#include "filters.hpp"
#include "sorters.hpp"

namespace analyzer
{
using sorters::sorter_ptr;
using filters::filter_ptr;
using namespace Entities;

class analyzer
{
public:
	analyzer(Global * data_)
		:data(data_)
	{}

	void analyze_profit();
	void dump_treasures();

private:
	void calc_profits();
	void calc_profits(filter_ptr filt, sorter_ptr sorter);
	filter_ptr createPathFilter();
	filter_ptr createGoodsFilter();
	filter_ptr createRadiusFilter();
	filter_ptr createFilter();
	sorter_ptr createSort();

	Global * data = nullptr;
};

} //namespace analyzer