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
		std::optional<std::string> star_to;
		std::optional<std::string> planet_from;
		std::optional<std::string> planet_to;
		std::optional<int>         tops_count;
	};

	inline Options& get_opt()
	{
		static Options opt {};
		return opt;
	}
}