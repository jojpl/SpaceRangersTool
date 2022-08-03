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

		bool operator()(const TradeInfo& pr1, const TradeInfo& pr2) const override
		{
			return pr1.profit.delta_profit < pr2.profit.delta_profit;
		}
	};

	// any field of any struct less than other
	template<typename T, typename Ret, typename TS, typename RetS>
	struct CommonSorter : ISort
	{
		CommonSorter(RetS TS::* struc, Ret T::* field)
			: struct_(struc)
			, field_ (field)
		{	}

		Ret T::* field_;
		RetS TS::* struct_;

		bool operator()(const TradeInfo& pr1, const TradeInfo& pr2) const override
		{
			return (pr1.*struct_).*field_ < (pr2.*struct_).*field_;
		}
	};

	//struct DistanceSorter : ISort
	//{
	//	DistanceSorter() = default;

	//	bool operator()(const TradeInfo& pr1, const TradeInfo& pr2) const override
	//	{
	//		return pr1.path.distance < pr2.path.distance;
	//	}
	//};

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


	struct AndSorter : ISort
	{
		AndSorter(sorter_ptr p1, sorter_ptr p2)
			: p1_(p1), p2_(p2)
		{	}

		sorter_ptr p1_;
		sorter_ptr p2_;

		bool operator()(const TradeInfo& ti1, const TradeInfo& ti2) const override
		{
			if (!p1_->operator()(ti1, ti2) && !p1_->operator()(ti2, ti1)) //eq
			{
				return p2_->operator()(ti1, ti2);
			}
			return p1_->operator()(ti1, ti2);
		}
	};
}