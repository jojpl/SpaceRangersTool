#pragma once
//#include "analyzer_entities.h"
#include <memory>
#include <functional>
#include <any>

namespace sorters
{
	//using namespace analyzer;
	using std::any;
	using std::any_cast;

	// common interface
	struct ISort
	{
		// less operator
		virtual bool operator()(const any&, const any&) const = 0;
		virtual ~ISort() = default;
	};

	using sorter_ptr = std::shared_ptr<ISort>;

	// help struct
	template<typename T>
	struct member_ptr;

	template<typename T, typename R>
	struct member_ptr<R T::*>
	{
		using type = T;
		using ret = R;
	};

	// any field of any struct less than other
	template<typename ... Args>
	struct CommonSorter : ISort
	{
		CommonSorter(Args ... others)
			: others_(others ...)
		{
			static_assert(std::conjunction_v< std::is_member_object_pointer<Args> ... >,
					"Use member_ptr's as Args");
		}

		using First = typename std::tuple_element_t<0, std::tuple<Args ...>>;
		using First_Ret = typename member_ptr<First>::ret;
		using First_T = typename member_ptr<First>::type;
		
		std::tuple<Args ...> others_; // member_ptr's

		bool operator()(const First_T& pr1, const First_T& pr2) const
		{
			return std::apply(
				[&pr1, &pr2](const Args& ... args){
					// unfold args for call chain of
					// ((pr1.*f1).*f2).*fn < ((pr2.*f1).*f2).*fn;
					return (pr1 .* ... .* args) < (pr2 .* ... .* args);
				}
			, others_); // unfold tuple to args ...
		}

		// for storaging
		virtual bool operator()(const any& a1, const any& a2) const override
		{
			return operator()(any_cast<First_T>(a1), any_cast<First_T>(a2));
		}
	};

	struct ASC_Wrapper : ISort
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

	struct AndSorter : ISort
	{
		AndSorter(sorter_ptr p1, sorter_ptr p2)
			: p1_(p1), p2_(p2)
		{	}

		sorter_ptr p1_;
		sorter_ptr p2_;

		bool operator()(const any& a1, const any& a2) const override
		{
			if (!(*p1_)(a1, a2) && !(*p1_)(a2, a1)) //eq
			{
				return (*p2_)(a1, a2);
			}
			return (*p1_)(a1, a2);
		}
	};
}