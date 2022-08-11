#include "programargs.hpp"
#include "common_algo.h"

#include <string>
#include <iostream>
#include <istream>
#include <ostream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

namespace po = boost::program_options;
namespace options 
{
	using namespace std::placeholders;
	po::variables_map vm;
	po::options_description desc("Allowed options");

	bool parse_args(int argc, char *argv[])
	{
		try {

			desc.add_options()
				("help,h", "produce help message")

				("from,f", po::value<std::string>()->notifier(handle_star_from), "star-from descr")
				("to,t", po::value<std::string>()->notifier(handle_star_to), "star-to descr")
				("from-planet", po::value<std::string>()->notifier(handle_planet_from), "planet-from descr")
				("to-planet", po::value<std::string>()->notifier(handle_planet_to), "planet-to descr")
				("max-dist,d", po::value<int>()->default_value(40), "max-dist descr")
				("count", po::value<std::string>()->default_value("10")->notifier(handle_count), "top's count also aviable \"all\" value")
				("min-profit,p", po::value<int>()->default_value(1000), "min-profit descr")
				("sort-by,s", po::value<std::string>()->default_value("profit")
					->notifier(handle_sort_options),
						"sort \"1[,2:[asc:desc] .. ]\"\n"
						"aviable fields:\n"
						"profit, distance,star, planet, good\n"
						"use quotes!")
				("goods,g", po::value<std::vector<std::string>>()->composing()->notifier(handle_goods),
						"goods include list")
				("goods-no", po::value<std::vector<std::string>>()->composing()->notifier(handle_no_goods),
						"goods exclude list")
				("dir", po::value<std::string>(), "directory of save files")
				("radius,r", po::value<int>(), "search radius around player")
				("storage", po::value<std::string>()->notifier(handle_storage), "recalc result using aviable storage")
				("mod,m", po::value<Modes>()->default_value(Modes::profit), "show prices")
				("tops", po::value<bool>()->implicit_value(true), "show top deal for from each planet for each good")
				("perf", po::value<bool>()->implicit_value(true)->default_value(false), "enable performance tracker")
				// TODO fix to hidden opt
			;
			po::positional_options_description pd;
			pd.add("count", 1);
			
			auto parsed_options = po::command_line_parser(argc, argv)
				.options(desc)
				.positional(pd)
				.run();

			po::store(parsed_options, vm);
			
			//po::options_description cfg_desc("Allowed options");
			//cfg_desc.add_options()
			//	("gui.accessibility.visual_bell",po::value<bool>(), "produce help message")
			//	;
			//po::store(parse_config_file("cfg.txt", cfg_desc), vm);

			po::notify(vm);

			if (vm.count("help")) {
				std::cout << desc << "\n";
				return false;
			}

			#define SAVE_TO_OPTIONAL(name, alias) \
			if (vm.count(name)) get_opt().alias = vm[name].as<typename decltype(alias)::value_type>();
			#define SAVE_AS_IS(name, alias) \
			if (vm.count(name)) get_opt().alias = vm[name].as<decltype(alias)>();
			
			SAVE_TO_OPTIONAL("max-dist", Options::max_dist)
			SAVE_AS_IS("min-profit", Options::min_profit)
			//SAVE_OPTION_AS_IS("count", Options::count)
			SAVE_TO_OPTIONAL("sort-by", Options::sort_by)
			SAVE_TO_OPTIONAL("dir", Options::dir)
			SAVE_TO_OPTIONAL("radius", Options::search_radius)
			//SAVE_TO_OPTIONAL("storage", Options::aviable_storage)
			SAVE_AS_IS("mod", Options::mod)
			SAVE_AS_IS("tops", Options::tops)
			SAVE_AS_IS("perf", Options::perf)
		}
		catch (std::exception& e) {
			std::cerr << "error: " << e.what() << "\n";
			std::cout << desc << "\n";
			return false;
		}
		catch (...) {
			std::cerr << "Exception of unknown type!\n";
			return false;
		}

		return true;
	}

	void from_string(SortDirection& d, std::string_view sw)
	{
		auto pos = common_algo::soft_search(sw,
			{
				"DESC",
				"ASC"
			});

		switch (pos)
		{
			case 0: d = SortDirection::DESC; break;
			case 1: d = SortDirection::ASC; break;

			default: throw po::invalid_option_value(std::string{ sw });
		}
	}

	void from_string(SortField& f, std::string_view sw)
	{
		auto pos = common_algo::soft_search(sw, 
			{
				"profit",
				"distance",
				"star",
				"planet",
				"good"
			});

		switch (pos)
		{
			case 0: f = SortField::profit; break;
			case 1: f = SortField::distance; break;
			case 2: f = SortField::star; break;
			case 3: f = SortField::planet; break;
			case 4: f = SortField::good; break;

			default: throw po::invalid_option_value(std::string{sw});
		}
	}

	void from_string(SortOpt& f, std::string_view sw)
	{
		std::vector<std::string> split_param;
		boost::split(split_param, sw, std::bind(std::equal_to<char>(), _1, ':'));

		int cnt = 0;
		for (auto param : split_param)
		{
			boost::trim(param);

			if (cnt == 0)
			{
				from_string(f.first, param);
				f.second = SortDirection::DESC; //default
			}
			else if (cnt == 1)
				from_string(f.second, param);
			cnt++;
		}
	}

	void handle_sort_options(const std::string& val)
	{
		using tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;
		
		auto& sort_options = get_opt().sort_options;
		tokenizer csv_tok(val);
		for (const auto &t : csv_tok)
		{
			SortOpt opt;
			from_string(opt, t);

			sort_options.push_back(opt);
		}
	}

	void handle_star_from(const std::string& val)
	{
		auto& opt = get_opt();
		if (    boost::iequals(val, "current")
			||  boost::iequals(val, "cur")
			||  boost::iequals(val, "c") )
		{
			opt.star_from_use_current = true;
		}
		else
			opt.star_from = val;
	}

	void handle_star_to(const std::string& val)
	{
		auto& opt = get_opt();
		if (    boost::iequals(val, "current")
			||  boost::iequals(val, "cur")
			||  boost::iequals(val, "c") )
		{
			opt.star_to_use_current = true;
		}
		else
			opt.star_to = val;
	}

	void handle_planet_from(const std::string& val)
	{
		auto& opt = get_opt();
		if (    boost::iequals(val, "current")
			||  boost::iequals(val, "cur")
			||  boost::iequals(val, "c") )
		{
			opt.planet_from_use_current = true;
		}
		else
			opt.planet_from = val;
	}

	void handle_planet_to(const std::string& val)
	{
		auto& opt = get_opt();
		if (    boost::iequals(val, "current")
			||  boost::iequals(val, "cur")
			||  boost::iequals(val, "c") )
		{
			opt.planet_to_use_current = true;
		}
		else
			opt.planet_to = val;
	}

	void handle_count(const std::string& val)
	{
		auto& opt = get_opt();
		if (boost::iequals(val, "all"))
			opt.count = INT_MAX;
		else
		{
			try
			{
				opt.count = std::stoi(val);
			}
			catch(...)
			{
				boost::throw_exception(po::invalid_option_value(val));
			}
		}
	}

	void handle_goods(const std::vector<std::string>& vals)
	{
		auto& opt = get_opt();
		for (const auto& v : vals)
		{
			std::vector<std::string> split_param;
			boost::split(split_param, v, std::bind(std::equal_to<char>(), _1, ',')
			);

			for (auto& item : split_param)
			{
				boost::trim(item);
				opt.goods.push_back(item);
			}
		}
	}

	void handle_no_goods(const std::vector<std::string>& vals)
	{
		auto& opt = get_opt();

		for (const auto& v : vals)
		{
			std::vector<std::string> split_param;
			boost::split(split_param, v, std::bind(std::equal_to<char>(), _1, ',')
			);

			for (auto& item : split_param)
			{
				boost::trim(item);
				opt.no_goods.push_back(item);
			}
		}
	}

	// Option "mod,m"
	std::istream& operator>>(std::istream& os, options::Modes & ms)
	{
		std::string s;
		os >> s;

		auto pos = common_algo::soft_search(s, 
			{"profit", "price", "treasures", "holes", "ritch"}
		);

		switch (pos)
		{
			case 0: ms = Modes::profit; break;
			case 1: ms = Modes::price; break;
			case 2: ms = Modes::treasures; break;
			case 3: ms = Modes::holes; break;
			case 4: ms = Modes::ritch; break;

			default: os.setstate(std::istream::badbit); break;
		}
		return os;
	}
	
	std::ostream& operator<<(std::ostream& os, const options::Modes& ms)
	{
		return os;
	}

	void handle_storage(const std::string& val)
	{
		auto& opt = get_opt();
		if (boost::iequals(val, "current")
			|| boost::iequals(val, "cur")
			|| boost::iequals(val, "c"))
		{
			opt.aviable_storage_cur = true;
		}
		else
			opt.aviable_storage = boost::lexical_cast<int>(val);
	}
}
