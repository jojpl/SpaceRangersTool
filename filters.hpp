#pragma once
#include "analyzer_entities.h"
#include "programargs.hpp"

#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <iostream>
#include <optional>

namespace filters
{
	using namespace analyzer;
	struct IFilter;
	using filter_ptr = std::shared_ptr<IFilter>;

	struct IFilter
	{
		// true - accept, false - decline
		virtual bool operator()(const TradeInfo&) = 0;
		virtual ~IFilter() = default;
	};

	struct FilterByPathCommon : IFilter
	{
		FilterByPathCommon() {}

		const std::set<std::string> skip_star_list_name
		{ "Тортугац", "Нифигац" };

		const std::set<std::string> skip_star_list_owners
		{ "Klings" };

		const std::set<std::string> skip_planet_list_owner
		{ "None", "Kling" };

		bool operator()(const TradeInfo& ti) {
			auto s1 = ti.path.s1;
			auto s2 = ti.path.s2;
			auto p1 = ti.path.p1;
			auto p2 = ti.path.p2;

			if (skip_star_list_name.count(s1->StarName))
				return false;
			if (skip_star_list_name.count(s2->StarName))
				return false;

			if (skip_star_list_owners.count(s1->Owners))
				return false;
			if (skip_star_list_owners.count(s2->Owners))
				return false;

			if (skip_planet_list_owner.count(p1->Owner))
				return false;
			if (skip_planet_list_owner.count(p2->Owner))
				return false;

			return true;
		}
	};

	struct FilterCurStarFrom : IFilter
	{
		FilterCurStarFrom(int id)
			: s_from_id(id)
		{	}

		int s_from_id;

		bool operator()(const TradeInfo& ti) {
			auto* s1 = ti.path.s1;

			if (s1->Id != s_from_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurStarTo : IFilter
	{
		FilterCurStarTo(int id)
			: s_to_id(id)
		{	}

		int s_to_id;

		bool operator()(const TradeInfo& ti) {
			auto* s2 = ti.path.s2;

			if (s2->Id != s_to_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurPlanetFrom : IFilter
	{
		FilterCurPlanetFrom(int id)
			: p_from_id(id)
		{	}

		int p_from_id;

		bool operator()(const TradeInfo& ti) {
			auto* p1 = ti.path.p1;

			if (p1->Id != p_from_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurPlanetTo : IFilter
	{
		FilterCurPlanetTo(int id)
			: p_to_id(id)
		{	}

		int p_to_id;

		bool operator()(const TradeInfo& ti) {
			auto* p2 = ti.path.p2;

			if (p2->Id != p_to_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterByPath : IFilter
	{
		FilterByPath(
			int max_dist,
			int s1_id, int s2_id,
			int p1_id, int p2_id)
		: max_dist_(max_dist),
		  s1_id_(s1_id), s2_id_(s2_id),
		  p1_id_(p1_id), p2_id_(p2_id)
		{	}

		int max_dist_;
		int s1_id_;
		int s2_id_;
		int p1_id_;
		int p2_id_;

		bool operator()(const TradeInfo& ti) {
			auto s1 = ti.path.s1;
			auto s2 = ti.path.s2;
			auto p1 = ti.path.p1;
			auto p2 = ti.path.p2;

			if (ti.path.distance > max_dist_)
				return false;

			if (s1_id_ && s1->Id != s1_id_)
				return false;

			if (s2_id_ && s2->Id != s2_id_)
				return false;

			if (p1_id_ && p1->Id != p1_id_)
				return false;

			if (p2_id_ && p2->Id != p2_id_)
				return false;

			return true;
		}
	};

	struct FilterByMinProfit : IFilter
	{
		FilterByMinProfit(options::Options opt)
			: min_profit_(opt.min_profit.value())
		{	}

		int min_profit_;

		bool operator()(const TradeInfo& ti) {
			if (ti.profit.delta_profit < min_profit_)
				return false;

			return true;
		}
	};

	struct NOT_opt : IFilter
	{
		NOT_opt(filter_ptr f)
			: f_(f)
		{	}

		filter_ptr f_;
		bool operator()(const TradeInfo& ti) {
			return !(*f_)(ti);
		}
	};

	struct FilterGoods : IFilter
	{
		FilterGoods(std::set<GoodsEnum> g)
			: g_(g)
		{	}

		const std::set<GoodsEnum> g_;

		// true - accept, false - decline
		bool operator()(const TradeInfo& ti) {
			return g_.count(ti.profit.good);
		}
	};

	// some template magic
	template <typename ... Args>
	struct AND_opt : IFilter
	{
		AND_opt(std::shared_ptr<Args> ... args)
			: filters(args...)
		{	}

		std::tuple<std::shared_ptr<Args> ...> filters;

		// aka template labmda c++20
		struct Help_Me
		{
			const TradeInfo& ti;

			template<typename ... Args>
			bool operator()(std::shared_ptr<Args>& ... args)
			{
				// unfold to call for each (arg1(pr) && ... && argN(pr)), stop if false
				bool res = ((*args)(ti) && ...);
				if (res)
					return true;

				return false;
			}
		};

		bool operator()(const TradeInfo& ti) {
			return std::apply(Help_Me{ ti }, filters); // unfold tuple to args ...
		}
	};
} // namespace filters

