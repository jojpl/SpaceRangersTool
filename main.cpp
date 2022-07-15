#include <iostream>

#include <SDKDDKVer.h>
#include <windows.h>
#include <debugapi.h>

#include "parser.h"

int main(int argc, char *argv[])
{
	SetConsoleOutputCP(1251);

	Entities::kv::init_storage();
	//DebugBreak();
	std::wstring file = L"C:\\Users\\Administrator.LW7S\\Documents\\spacerangershd\\save\\"
	L"Планета Палтеомео-dump.txt";

	std::string mem;
	if (!read_file(mem, file))
	{
		return -1;
	}
	extern void some(const std::string& mem);
	//some(mem);
	try
	{
		parse(mem);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
	}
}