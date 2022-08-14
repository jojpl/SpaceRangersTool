#pragma once

#include <optional>
#include <iosfwd>
#include <string>
#include <vector>
#include <set>

namespace options
{
	using namespace std::string_literals;
	enum class SortDirection
	{
		DESC,
		ASC,
	};

	enum class SortField
	{
		profit,
		distance,
		star,
		planet,
		good
	};

	using SortOpt     = std::pair<SortField, SortDirection>;
	using SortOptions = std::vector<SortOpt>;

	enum class Modes
	{
		profit,
		price,
		treasures,
		holes,
		ritch,
	};

	struct Options
	{
		std::optional<int>         max_dist;
		int                        min_profit = 0;
		std::optional<std::string> star_from;
		bool star_from_use_current = false;
		std::optional<std::string> star_to;
		bool star_to_use_current   = false;
		std::optional<std::string> planet_from;
		bool planet_from_use_current = false;
		std::optional<std::string> planet_to;
		bool planet_to_use_current = false;
		std::optional<int>         count;
		std::optional<std::string> sort_by;
		SortOptions                sort_options;
		std::vector<std::string>   goods;
		std::vector<std::string>   no_goods;
		std::optional<std::string> dir;
		std::optional<int>         search_radius;
		std::optional<int>         aviable_storage;
		bool                       aviable_storage_cur = false;
		Modes                      mod = Modes::profit;
		bool                       tops = false;
		bool                       perf = false;
		struct ItemSearch
		{
			std::optional<std::string> IType;
			std::optional<std::string> IName;
			std::optional<std::string> Size;
			std::optional<std::string> MinDamage;
			std::optional<std::string> TechLevel;
		};
		std::optional<ItemSearch>  itemSearch;
	};

	inline Options& get_opt()
	{
		static Options opt{};
		return opt;
	}

	bool parse_args(int argc, char *argv[]);

	static void handle_sort_options(const std::string& val);
	static void handle_star_from(const std::string& val);
	static void handle_star_to(const std::string& val);
	static void handle_planet_from(const std::string& val);
	static void handle_planet_to(const std::string& val);
	static void handle_goods(const std::vector<std::string>& val);
	static void handle_count(const std::string& val);
	static void handle_no_goods(const std::vector<std::string>& val);
	static void handle_storage(const std::string& val);

	std::istream& operator>>(std::istream&, Modes&);
	std::ostream& operator<<(std::ostream&, const Modes&);
}
