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
	{
		print_game_date();
	}

	void analyze_profit();
	void dump_treasures();
	void show_price();
	void dump_holelist();
	void print_game_date();
	void show_ritches();

private:
	void calc_profits();
	void calc_all_trade_paths_info(std::vector<TradeInfo>&);
	void calc_profits(filter_ptr filt, sorter_ptr sorter);
	filter_ptr createPathFilter();
	filter_ptr createGoodsFilter();
	filter_ptr createRadiusFilter();
	filter_ptr createFilter();
	sorter_ptr createSort();

	Global * data = nullptr;
	sorter_ptr tops_cmp_{};
};

int get_distance(const Star* s1, const Star* s2);
int get_distance(const Location& loc1, const Location& loc2);

} //namespace analyzer