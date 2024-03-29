﻿#include "parser.h"
#include "analyzer.hpp"
#include "filefinder.hpp"
#include "programargs.hpp"

#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std::string_literals;

bool read_file_as_mem(std::string& out, const std::string& path)
{
	std::ifstream f(path, std::ifstream::binary);
	if (!f) return false;

	f.seekg(0, std::ifstream::end);
	auto len = (size_t)f.tellg();
	f.seekg(0);

	std::vector<char> buf;
	buf.resize(len);

	f.read(buf.data(), len);

	out.assign(buf.begin(), buf.end());
	return !out.empty();
}

void on_new_file_found(std::string file)
{
	try
	{
		std::string mem;
		if(!read_file_as_mem(mem, file))
		throw std::logic_error("Can't read file : "s + file);

		Entities::Global* out = parser::parse(mem);
		mem.resize(0);
		
		analyzer::analyzer a(out);
		const auto& opt = options::get_opt();
		switch (opt.mod)
		{
			case options::Modes::price:
				a.show_price();
				break;

			case options::Modes::profit:
				a.analyze_profit();
				break;

			case options::Modes::treasures:
				a.dump_treasures();
				break;			
			
			case options::Modes::holes:
				a.dump_holelist();
				break;			
				
			case options::Modes::ritch:
				a.show_ritches();
				break;

			default:
				break;
		}

		if(opt.itemSearch)
			a.show_items_search_reminder();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
		return;
	}
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
	::SetConsoleOutputCP(1251);
	//::SetConsoleTextAttribute()
	//_get_osfhandle
#endif
	if(!options::parse_args(argc, argv))
		return 0;

	filefinder fh;
	fh.set_handler(on_new_file_found);
	fh.find();
}