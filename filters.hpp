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

	struct IFilter
	{
		// true - accept, false - decline
		virtual bool operator()(const Profit&) = 0;
		virtual ~IFilter() = default;
	};

	using filter_ptr = std::shared_ptr<IFilter>;

	struct FilterByPathCommon : IFilter
	{
		FilterByPathCommon() {}
		virtual ~FilterByPathCommon() = default;

		const std::set<std::string> skip_star_list_name
		{ "Тортугац", "Нифигац" };

		const std::set<std::string> skip_star_list_owners
		{ "Klings" };

		const std::set<std::string> skip_planet_list_owner
		{ "None", "Kling" };

		bool operator()(const Profit& pr) {
			auto s1 = pr.path.s1;
			auto s2 = pr.path.s2;
			auto p1 = pr.path.p1;
			auto p2 = pr.path.p2;

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
		{
		}

		int s_from_id;

		bool operator()(const Profit& pr) {
			auto* s1 = pr.path.s1;

			if (s1->Id != s_from_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurStarTo : IFilter
	{
		FilterCurStarTo(int id)
			: s_to_id(id)
		{
		}

		int s_to_id;

		bool operator()(const Profit& pr) {
			auto* s2 = pr.path.s2;

			if (s2->Id != s_to_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurPlanetFrom : IFilter
	{
		FilterCurPlanetFrom(int id)
			: p_from_id(id)
		{
		}

		int p_from_id;

		bool operator()(const Profit& pr) {
			auto* p1 = pr.path.p1;

			if (p1->Id != p_from_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurPlanetTo : IFilter
	{
		FilterCurPlanetTo(int id)
			: p_to_id(id)
		{
		}

		int p_to_id;

		bool operator()(const Profit& pr) {
			auto* p2 = pr.path.p2;

			if (p2->Id != p_to_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterByPath : IFilter
	{
		FilterByPath(options::Options opt)
			: opt_(opt)
		{
		}

		options::Options opt_;

		bool operator()(const Profit& pr) {
			auto s1 = pr.path.s1;
			auto s2 = pr.path.s2;
			auto p1 = pr.path.p1;
			auto p2 = pr.path.p2;

			if (pr.path.distance > opt_.max_dist.value())
				return false;

			if (opt_.star_from && s1->StarName != opt_.star_from.value())
				return false;

			if (opt_.star_to && s2->StarName != opt_.star_to.value())
				return false;

			if (opt_.planet_from && p1->PlanetName != opt_.planet_from.value())
				return false;

			if (opt_.planet_to && p2->PlanetName != opt_.planet_to.value())
				return false;

			return true;
		}
	};

	struct FilterByPath_v2 : IFilter
	{
		FilterByPath_v2(
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

		bool operator()(const Profit& pr) {
			auto s1 = pr.path.s1;
			auto s2 = pr.path.s2;
			auto p1 = pr.path.p1;
			auto p2 = pr.path.p2;

			if (pr.path.distance > max_dist_)
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

	struct FilterByProfit : IFilter
	{
		FilterByProfit(options::Options opt)
			: min_profit_(opt.min_profit.value())
		{	}

		virtual ~FilterByProfit() = default;

		int min_profit_;

		bool operator()(const Profit& pr) {
			if (pr.delta_profit < min_profit_)
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
		bool operator()(const Profit& pr) {
			return !(*f_)(pr);
		}
	};

	struct FilterGoods : IFilter
	{
		FilterGoods(std::set<Entities::GoodsEnum> g)
			: g_(g)
		{	}

		const std::set<Entities::GoodsEnum> g_;

		// true - accept, false - decline
		bool operator()(const Profit& pr) {
			return g_.count(pr.good);
		}
	};

	// some template magic
	template <typename ... Args>
	struct AND_opt : IFilter
	{
		//AND_opt(std::shared_ptr<Args>&& ... args)
		//	: filters( std::forward<std::shared_ptr<Args>>(args)... )
		//{	}

		AND_opt(std::shared_ptr<Args> ... args)
			: filters(args...)
		{	}

		virtual ~AND_opt() = default;

		std::tuple<std::shared_ptr<Args> ...> filters;

		// aka template labmda c++20
		struct Help_Me
		{
			const Profit& pr;

			template<typename ... Args>
			bool operator()(std::shared_ptr<Args>& ... args)
			{
				// unfold to call for each (arg1(pr) && ... && argN(pr)), stop if false
				bool res = ((*args)(pr) && ...);
				if (res)
					return true;

				return false;
			}
		};

		bool operator()(const Profit& pr) {
			return std::apply(Help_Me{ pr }, filters); // unfold tuple to args ...
		}

	};
}

