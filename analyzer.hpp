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

struct IFilter
{
	// true - accept, false - decline
	virtual bool operator()(Profit&) = 0;
	~IFilter() = default;
};

struct ISort
{
	// less operator
	virtual bool operator()(Profit&, Profit&) const = 0;
	~ISort() = default;
};

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

