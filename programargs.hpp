#pragma once

#include <optional>
#include <string>

namespace options
{
	bool parse_args(int argc, char *argv[]);

	struct Options
	{
		std::optional<int>         max_dist;
		std::optional<int>         min_profit;
		std::optional<std::string> star_from;
		bool star_from_use_current = false; //hidden opt
		std::optional<std::string> star_to;
		std::optional<std::string> planet_from;
		bool planet_from_use_current = false; //hidden opt
		std::optional<std::string> planet_to;
		std::optional<int>         count;
		std::optional<std::string> sort_by;
	};

	inline Options& get_opt()
	{
		static Options opt {};
		return opt;
	}
}