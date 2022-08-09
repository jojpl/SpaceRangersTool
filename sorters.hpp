#pragma once
#include "analyzer_entities.h"
#include <memory>
#include <functional>
#include <any>

namespace sorters
{
	using namespace analyzer;
	using std::any;
	using std::any_cast;

	struct ISort_v2
	{
		// less operator
		virtual bool operator()(const any&, const any&) const = 0;
		virtual ~ISort_v2() = default;
	};

	using sorter_ptr = std::shared_ptr<ISort_v2>;

	// help struct
	template<typename ... Args>
	struct mem_ptr {};

	template<typename T, typename R>
	struct mem_ptr<R T::*>
	{
		using type = T;
		using ret = R;
	};

	template<typename F>
	struct mem_ptr<F> : mem_ptr<F> // like a virtual fn
	{};

	// any field of any struct less than other
	template<typename T, typename RetT,
			typename ... Args>
	struct CommonSorter : ISort_v2
	{
		CommonSorter(RetT T::* first, Args ... others)
			: f0_(first)
			, others_(others ...)
		{	}

		RetT T::* f0_;
		std::tuple<Args ...> others_;

		bool operator()(const T& pr1, const T& pr2) const
		{
			return std::apply
			(
				[&pr1, &pr2, this](const Args& ... args){
						// unfold typle for call
			            // ((pr1.*struct_).*field_).*field3_ < ((pr2.*struct_).*field_).*field3_;
						auto res1 = ((pr1.*f0_) .* ... .* args);
						auto res2 = ((pr2.*f0_) .* ... .* args);
						return res1 < res2;
					}
			, others_); // unfold tuple to args ...
			
		}

		// for storaging
		virtual bool operator()(const any& a1, const any& a2) const override
		{
			return operator()(any_cast<T>(a1), any_cast<T>(a2));
		}
	};

	struct MaxProfitSorter : CommonSorter<TradeInfo, Profit, int Profit::* >
	{
		MaxProfitSorter() : CommonSorter(&TradeInfo::profit, &Profit::delta_profit) {}
	};

	struct MaxDistanceSorter : CommonSorter<TradeInfo, Path, int Path::*>
	{
		MaxDistanceSorter() : CommonSorter(&TradeInfo::path, &Path::distance) {}
	};

	struct ASC_Wrapper : ISort_v2
	{
		ASC_Wrapper(sorter_ptr obj_)
			: obj(obj_)
		{	}

		sorter_ptr obj;
		bool operator()(const any& a1, const any& a2) const override
		{
			return (*obj)(a2, a1); //revert
		}
	};

	struct AndSorter : ISort_v2
	{
		AndSorter(sorter_ptr p1, sorter_ptr p2)
			: p1_(p1), p2_(p2)
		{	}

		sorter_ptr p1_;
		sorter_ptr p2_;

		bool operator()(const any& a1, const any& a2) const override
		{
			if (!p1_->operator()(a1, a2) && !p1_->operator()(a2, a1)) //eq
			{
				return p2_->operator()(a1, a2);
			}
			return p1_->operator()(a1, a2);
		}
	};
}