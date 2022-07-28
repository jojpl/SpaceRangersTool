#pragma once

#include "filters.hpp"
#include "analyzer_entities.h"
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
	void calc_profits(std::shared_ptr<IFilter> filt);
	std::shared_ptr<IFilter> createFilter();
	std::shared_ptr<ISort>   createSort();

	Entities::Global * data = nullptr;
	std::shared_ptr<ISort> sorter_;
};

