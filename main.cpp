#include "parser.h"
#include "analyzer.hpp"
#include "filefinder.hpp"

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
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
		return;
	}
	analyzer(out).analyze_profit();
}

int main(int argc, char *argv[])
{
	::SetConsoleOutputCP(1251);

	filefinder fh;
	fh.set_handler(on_new_file_found);
	fh.find();
}