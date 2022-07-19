#include <iostream>

#include <SDKDDKVer.h>
#include <windows.h>
#include <chrono>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
class MemStateRAII
{
	_CrtMemState _ms;
public:
	MemStateRAII() { _CrtMemCheckpoint(&_ms); }
	~MemStateRAII() { _CrtMemDumpAllObjectsSince(&_ms); }
} gfgf;

#include "parser.h"

int main(int argc, char *argv[])
{
	std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
	::SetConsoleOutputCP(1251);

	model::kv::init_storage();
	//DebugBreak();
	//std::wstring file = L"C:\\Users\\Administrator.LW7S\\Documents\\spacerangershd\\save\\"
	//L"Планета Палтеомео-dump.txt";

	std::string file = "C:\\Users\\Administrator.LW7S\\Documents\\spacerangershd\\save\\"
		"Планета Палтеомео-dump.txt";

	std::string mem;
	//if (!read_file(mem, file))
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

	auto now = std::chrono::steady_clock::now();
	std::cout << 
		std::chrono::duration_cast<std::chrono::milliseconds>(now - tp).count() 
		<<" ms";
}