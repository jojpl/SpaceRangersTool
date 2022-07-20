#include <iostream>

#include <chrono>

#include "parser.h"

int main(int argc, char *argv[])
{
	std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();

	model::kv::init_storage();
	//DebugBreak();
	//std::wstring file = L"C:\\Users\\Administrator.LW7S\\Documents\\spacerangershd\\save\\"
	//L"Планета Палтеомео-dump.txt";

	std::string file = "C:\\Users\\Administrator.LW7S\\Documents\\spacerangershd\\save\\"
		"Планета Палтеомео-dump.txt";

	try
	{
		std::string mem;
		if(read_file(mem, file))
			parse(mem);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
	}

	auto now = std::chrono::steady_clock::now();
	std::cout << 
		std::chrono::duration_cast<std::chrono::milliseconds>(now - tp).count() 
		<<" ms";
}