﻿#include "parser.h"
#include "analyzer.hpp"
#include "filefinder.hpp"
#include "programargs.hpp"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

void on_new_file_found(std::string file)
{
	Entities::Global* out = nullptr;
	try
	{
		std::string mem;
		if(parser::read_file(mem, file))
			out = parser::parse(mem);
		mem.clear();

		analyzer::analyzer(out).analyze_profit();
		//analyzer::analyzer(out).dump_treasures();
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
#endif

	if(!options::parse_args(argc, argv))
		return 0;

	filefinder fh;
	fh.set_handler(on_new_file_found);
	fh.find();
}