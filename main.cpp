#include "parser.h"
#include "analyzer.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	std::string file = "C:\\Users\\Administrator.LW7S\\Documents\\spacerangershd\\save\\"
		"Планета Палтеомео-dump.txt";
		
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
	}
	analyzer().analyze_some(out);
}