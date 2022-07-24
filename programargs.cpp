#include "programargs.hpp"

#include <boost\program_options.hpp>
#include <string>
#include <iostream>

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
				("max-dist", po::value<int>(), "max-dist descr")
				("min-profit", po::value<int>(), "min-profit descr")
				("star-from", po::value<std::string>(), "star-from descr")
				("star-to", po::value<std::string>(), "star-to descr")
				("planet-from", po::value<std::string>(), "planet-from descr")
				("planet-to", po::value<std::string>(), "planet-to descr")
				;

			po::store(po::parse_command_line(argc, argv, desc), vm);
			po::notify(vm);

			if (vm.count("help")) {
				std::cout << desc << "\n";
				return false;
			}

			#define SAVE_OPTION(name, alias) \
			if (vm.count(name)) opt.alias = vm[name].as<typename decltype(alias)::value_type>();

			SAVE_OPTION("max-dist", Options::max_dist)
			SAVE_OPTION("min-profit", Options::min_profit)
			SAVE_OPTION("star-from", Options::star_from)
			SAVE_OPTION("star-to", Options::star_to)
			SAVE_OPTION("planet-from", Options::planet_from)
			SAVE_OPTION("planet-to", Options::planet_to)
			#undef SAVE_OPTION
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