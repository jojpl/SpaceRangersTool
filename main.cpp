#include "parser.h"
#include "analyzer.hpp"
#include "filefinder.hpp"
#include "programargs.hpp"

#include <iostream>
#include <windows.h>

void on_new_file_found(std::string file)
{
	Entities::Global* out = nullptr;
	try
	{
		std::string mem;
		if(read_file(mem, file))
			out = parse(mem);
		analyzer(out).analyze_profit();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
		return;
	}
}

int main(int argc, char *argv[])
{
	//extern void Db_test();
	//Db_test();
	//return 0;

	::SetConsoleOutputCP(1251);

	if(!options::parse_args(argc, argv))
		return 0;

	filefinder fh;
	fh.set_handler(on_new_file_found);
	fh.find();
}