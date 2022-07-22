#pragma once
#include "model.hpp"

class analyzer
{
	Entities::Global * data = nullptr;
public:
	analyzer(Entities::Global * data_)
		:data(data_)
	{}

	void draw_stars_ASCII_pic();
	void analyze_profit();
};

