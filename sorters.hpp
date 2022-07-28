#pragma once
#include "analyzer_entities.h"
#include <memory>

namespace sorters
{
	struct ISort
	{
		// less operator
		virtual bool operator()(Profit&, Profit&) const = 0;
		virtual ~ISort() = default;
	};

	using sorter_ptr = std::shared_ptr<ISort>;

	struct DefaultSorter : ISort
	{
		DefaultSorter() = default;
		virtual ~DefaultSorter() = default;

		bool operator()(Profit& pr1, Profit& pr2) const override
		{
			return pr1.delta_profit < pr2.delta_profit;
		}
	};

	struct ProfitSorter : ISort
	{
		ProfitSorter() = default;
		virtual ~ProfitSorter() = default;

		bool operator()(Profit& pr1, Profit& pr2) const override
		{
			return pr1.delta_profit < pr2.delta_profit;
		}
	};

	struct ASC_Sort_Wrapper : ISort
	{
		ASC_Sort_Wrapper(sorter_ptr obj_)
			: obj(obj_)
		{	}

		sorter_ptr obj;
		bool operator()(Profit& pr1, Profit& pr2) const override
		{
			return (*obj)(pr2, pr1); //revert
		}
	};

	struct DistanceSorter : ISort
	{
		DistanceSorter() = default;
		virtual ~DistanceSorter() = default;

		bool operator()(Profit& pr1, Profit& pr2) const override
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

		bool operator()(Profit& pr1, Profit& pr2) const override
		{
			if (!p1_->operator()(pr1, pr2) && !p1_->operator()(pr2, pr1)) //eq
			{
				return p2_->operator()(pr1, pr2);
			}
			return p1_->operator()(pr1, pr2);
		}
	};
}