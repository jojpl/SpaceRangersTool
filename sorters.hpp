#pragma once
#include "analyzer_entities.h"
#include <memory>

namespace sorters
{
	using namespace analyzer;
	struct ISort;
	struct MaxProfitSorter;
	using sorter_ptr = std::shared_ptr<ISort>;
	using DefaultSorter = MaxProfitSorter;

	struct ISort
	{
		// less operator
		virtual bool operator()(const TradeInfo&, const TradeInfo&) const = 0;
		virtual ~ISort() = default;
	};

	struct MaxProfitSorter : ISort
	{
		MaxProfitSorter() = default;
		virtual ~MaxProfitSorter() = default;

		bool operator()(const TradeInfo& pr1, const TradeInfo& pr2) const override
		{
			return pr1.profit.delta_profit < pr2.profit.delta_profit;
		}
	};

	struct ASC_Wrapper : ISort
	{
		ASC_Wrapper(sorter_ptr obj_)
			: obj(obj_)
		{	}

		sorter_ptr obj;
		bool operator()(const TradeInfo& pr1, const TradeInfo& pr2) const override
		{
			return (*obj)(pr2, pr1); //revert
		}
	};

	struct DistanceSorter : ISort
	{
		DistanceSorter() = default;
		virtual ~DistanceSorter() = default;

		bool operator()(const TradeInfo& pr1, const TradeInfo& pr2) const override
		{
			return pr1.path.distance < pr2.path.distance;
		}
	};

	struct AndSorter : ISort
	{
		AndSorter(sorter_ptr p1, sorter_ptr p2)
			: p1_(p1), p2_(p2)
		{	}
		virtual ~AndSorter() = default;

		sorter_ptr p1_;
		sorter_ptr p2_;

		bool operator()(const TradeInfo& pr1, const TradeInfo& pr2) const override
		{
			if (!p1_->operator()(pr1, pr2) && !p1_->operator()(pr2, pr1)) //eq
			{
				return p2_->operator()(pr1, pr2);
			}
			return p1_->operator()(pr1, pr2);
		}
	};
}