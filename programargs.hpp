#pragma once

#include <optional>
#include <string>
#include <vector>
#include <set>

namespace options
{
	using namespace std::string_literals;
	enum class SortDirection
	{
		unknown,

		DESC,
		ASC,
	};

	enum class SortField
	{
		unknown,

		profit,
		distance,
	};

	using SortOpt     = std::pair<SortField, SortDirection>;
	using SortOptions = std::vector<SortOpt>;

	struct Options
	{
		std::optional<int>         max_dist;
		std::optional<int>         min_profit;
		std::optional<std::string> star_from;
		bool star_from_use_current = false; //hidden opt
		std::optional<std::string> star_to;
		bool star_to_use_current   = false; //hidden opt
		std::optional<std::string> planet_from;
		bool planet_from_use_current = false; //hidden opt
		std::optional<std::string> planet_to;
		bool planet_to_use_current = false; //hidden opt
		std::optional<int>         count;
		std::optional<std::string> sort_by;
		SortOptions                sort_options;
		std::vector<std::string>   goods;
		std::vector<std::string>   no_goods;
		std::optional<std::string> dir;
		std::optional<int>         search_radius;
		std::optional<int>         aviable_storage;
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
	static void handle_no_goods(const std::vector<std::string>& val);
	static void handle_dir(const std::string& val);
}