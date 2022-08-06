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

	struct IPathFilter : IFilter
	{
		virtual bool operator()(const Path&) = 0;
		virtual ~IPathFilter() = default;

		//private:
		virtual bool operator()(const TradeInfo& ti) override
		{
			return operator()(ti.path);
		}

	};

	struct IProfitFilter : IFilter
	{
		virtual bool operator()(const Profit&) = 0;
		virtual ~IProfitFilter() = default;

		//private:
		virtual bool operator()(const TradeInfo& ti) override
		{
			return operator()(ti.profit);
		}
	};

	struct FilterByPathCommon : IPathFilter
	{
		FilterByPathCommon() {}

		const std::set<std::string> skip_star_list_name
		{ "Тортугац", "Нифигац" };

		const std::set<std::string> skip_star_list_owners
		{ "Klings" };

		const std::set<std::string> skip_planet_list_owner
		{ "None", "Kling" };

		bool operator()(const Path& path) override {
			auto* s1 = path.from.star;
			auto* s2 = path.to.star;
			auto* p1 = path.from.planet;
			auto* p2 = path.from.planet;

			if (s1 && skip_star_list_name.count(s1->StarName))
				return false;
			if (s2 && skip_star_list_name.count(s2->StarName))
				return false;

			if (s1 && skip_star_list_owners.count(s1->Owners))
				return false;
			if (s2 && skip_star_list_owners.count(s2->Owners))
				return false;

			if (p1 && skip_planet_list_owner.count(p1->Owner))
				return false;
			if (p2 && skip_planet_list_owner.count(p2->Owner))
				return false;

			return true;
		}
	};

	struct FilterCurStarFrom : IPathFilter
	{
		FilterCurStarFrom(int id)
			: s_from_id(id)
		{	}

		int s_from_id;

		bool operator()(const Path& path) override {
			auto* s1 = path.from.star;

			if (s1->Id != s_from_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurStarTo : IPathFilter
	{
		FilterCurStarTo(int id)
			: s_to_id(id)
		{	}

		int s_to_id;

		bool operator()(const Path& path) override {
			auto* s2 = path.to.star;

			if (s2->Id != s_to_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurPlanetFrom : IPathFilter
	{
		FilterCurPlanetFrom(int id)
			: p_from_id(id)
		{	}

		int p_from_id;

		bool operator()(const Path& path) override {
			auto* p1 = path.from.planet;

			if (p1->Id != p_from_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct FilterCurPlanetTo : IPathFilter
	{
		FilterCurPlanetTo(int id)
			: p_to_id(id)
		{	}

		int p_to_id;

		bool operator()(const Path& path) override {
			auto* p2 = path.to.planet;

			if (p2->Id != p_to_id) // faster than name cmp
				return false;
			return true;
		}
	};

	struct Nul_Opt : IFilter
	{
		Nul_Opt() = default;
		bool operator()(const TradeInfo&) override { return true; }
	};

	struct FilterByStarFromIdArr : IPathFilter
	{
		FilterByStarFromIdArr(const std::vector<int>& s1_ids)
			: s1_ids_(s1_ids)
		{	}

		const std::vector<int> s1_ids_;

		bool operator()(const Path& path) override {
			if (std::find(cbegin(s1_ids_), cend(s1_ids_), path.from.star->Id) != cend(s1_ids_))
				return true;
			return false;
		}
	};

	struct FilterByPath : IPathFilter
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

		bool operator()(const Path& path) override {
			auto* s1 = path.from.star;
			auto* s2 = path.to.star;
			auto* p1 = path.from.planet;
			auto* p2 = path.to.planet;

			if (path.distance > max_dist_)
				return false;

			if (s1_id_ && s1->Id != s1_id_)
				return false;

			if (s2_id_ && s2->Id != s2_id_)
				return false;

			if (p1_id_ && p1 && p1->Id != p1_id_)
				return false;

			if (p2_id_ && p2 && p2->Id != p2_id_)
				return false;

			return true;
		}
	};

	struct FilterByMinProfit : IProfitFilter
	{
		using IProfitFilter::operator(); //fix
		FilterByMinProfit(int min_profit)
			: min_profit_(min_profit)
		{	}

		int min_profit_;
		bool operator()(const Profit& profit) override {
			if (profit.delta_profit < min_profit_)
				return false;

			return true;
		}
	};

	template<typename T>
	struct NOT_opt : IFilter
	{
		NOT_opt(std::shared_ptr<T> f)
			: f_(f)
		{	}
		std::shared_ptr<T> f_;

		bool operator()(const TradeInfo& ti) override {
			return !(*f_)(ti);
		}
	};

	struct FilterGoods : IProfitFilter
	{
		FilterGoods(const std::set<GoodsEnum>& g)
			: g_(g)
		{	}

		const std::set<GoodsEnum> g_;

		bool operator()(const Profit& profit) {
			return g_.count(profit.good);
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

		bool operator()(const TradeInfo& ti) {
			return std::apply
			(
				[&ti](std::shared_ptr<Args>& ... args){
						// unfold typle for call each (arg1(pr) && ... && argN(pr)),
						// stop if false
						return ((*args)(ti) && ...);
					}
			, filters); // unfold tuple to args ...
		}
	};
} // namespace filters

