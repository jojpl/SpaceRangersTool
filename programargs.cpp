#include "programargs.hpp"

#include <string>
#include <iostream>

#include "boost\algorithm\string\predicate.hpp"
#include "boost\program_options.hpp"

namespace po = boost::program_options;
namespace options 
{
	po::variables_map vm;

	bool parse_args(int argc, char *argv[])
	{
		try {

			po::options_description desc("Allowed options");
			desc.add_options()
				("help", "produce help message")

				("max-dist", po::value<int>()->default_value(40), "max-dist descr")
				("min-profit", po::value<int>()->default_value(1000), "min-profit descr")
				("star-from", po::value<std::string>(), "star-from descr")
				//("star-from-use-current", po::value<bool>()->default_value(false), "use player's current star")
				("star-to", po::value<std::string>(), "star-to descr")
				("planet-from", po::value<std::string>(), "planet-from descr")
				//("planet-from-use-current", po::value<bool>()->default_value(false), "use player's current planet")
				("planet-to", po::value<std::string>(), "planet-to descr")
				("count", po::value<int>()->default_value(10), "top's count descr")
				//("count-all", po::value<int>()->default_value(10), "top's count descr")
				("sort-by", po::value<std::string>()->default_value("profit"), "sort 1[,2 .. ]\n"
															"profit, distance, star-from ...\n"
															"use quotes!")
				;

			po::store(po::parse_command_line(argc, argv, desc), vm);
			po::notify(vm);

			if (vm.count("help")) {
				std::cout << desc << "\n";
				return false;
			}

			#define SAVE_OPTION(name, alias) \
			if (vm.count(name)) get_opt().alias = vm[name].as<typename decltype(alias)::value_type>();

			SAVE_OPTION("max-dist", Options::max_dist)
			SAVE_OPTION("min-profit", Options::min_profit)
			SAVE_OPTION("star-from", Options::star_from)
			SAVE_OPTION("star-to", Options::star_to)
			SAVE_OPTION("planet-from", Options::planet_from)
			SAVE_OPTION("planet-to", Options::planet_to)
			SAVE_OPTION("count", Options::count)
			SAVE_OPTION("sort-by", Options::sort_by)
			#undef SAVE_OPTION
			auto& opt = get_opt();
			if (opt.star_from && (
					boost::iequals(opt.star_from.value(), "current")
				||  boost::iequals(opt.star_from.value(), "cur")))
			{
				opt.star_from_use_current = true;
				opt.star_from.reset();
			}
			if (opt.planet_from && (
					boost::iequals(opt.planet_from.value(), "current")
				||  boost::iequals(opt.planet_from.value(), "cur")))
			{
				opt.planet_from_use_current = true;
				opt.planet_from.reset();
			}
		}
		catch (std::exception& e) {
			std::cerr << "error: " << e.what() << "\n";
			return false;
		}
		catch (...) {
			std::cerr << "Exception of unknown type!\n";
			return false;
		}

		return true;
	}
}