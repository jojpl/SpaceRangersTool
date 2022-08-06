#pragma once
#include "analyzer_entities.h"
#include <memory>
#include <functional>

namespace sorters
{
	using namespace analyzer;

	template<typename T>
	struct ISort_v2
	{
		// less operator
		virtual bool operator()(const T&, const T&) const = 0;
		virtual ~ISort_v2() = default;
	};

	using sorter_ptr = std::shared_ptr<ISort_v2<TradeInfo>>;

	// any field of any struct less than other
	template<typename TS, typename RetS, 
			typename TS2, typename RetS2, 
			typename TS3, typename RetS3>
	struct CommonSorter3 : ISort_v2<TS>
	{
		CommonSorter3(RetS TS::* struc, RetS2 TS2::* field, RetS3 TS3::* field3)
			: struct_(struc)
			, field_(field)
			, field3_(field3)
		{	}

		RetS TS::*   struct_;
		RetS2 TS2::* field_;
		RetS3 TS3::* field3_;

		bool operator()(const TS& pr1, const TS& pr2) const override
		{
			return ((pr1.*struct_).*field_).*field3_ < ((pr2.*struct_).*field_).*field3_;
		}
	};

	template<typename TS, typename RetS, typename TS2, typename RetS2>
	struct CommonSorter2 : ISort_v2<TS>
	{
		CommonSorter2(RetS TS::* struc, RetS2 TS2::* field)
			: struct_(struc)
			, field_ (field)
		{	}

		RetS TS::*   struct_;
		RetS2 TS2::* field_;

		bool operator()(const TS& pr1, const TS& pr2) const override
		{
			return (pr1.*struct_).*field_ < (pr2.*struct_).*field_;
		}
	};

	template<typename T, typename R>
	struct mem_ptr
	{
		using Type = T ;
		using Ret  = R;
	};

	template<typename TS, typename RetS>
	struct CommonSorter1 : ISort_v2<TS>
	{
		CommonSorter1(RetS TS::* field)
			: field_(field)
		{	}

		RetS TS::*   field_;

		bool operator()(const TS& pr1, const TS& pr2) const override
		{
			return pr1.*field_ < pr2.*field_;
		}
	};

	struct MaxProfitSorter : CommonSorter2<TradeInfo, Profit, Profit, int>
	{
		MaxProfitSorter() : CommonSorter2(&TradeInfo::profit, &Profit::delta_profit) {}
	};

	struct MaxDistanceSorter : CommonSorter2<TradeInfo, Path, Path, int>
	{
		MaxDistanceSorter() : CommonSorter2(&TradeInfo::path, &Path::distance) {}
	};

	struct ASC_Wrapper : ISort_v2<TradeInfo>
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

	struct AndSorter : ISort_v2<TradeInfo>
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