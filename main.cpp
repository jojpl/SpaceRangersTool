#include <iostream>

#include <SDKDDKVer.h>
#include <windows.h>
#include <debugapi.h>

#include "parser.h"

int main(int argc, char *argv[])
{
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
    parse(mem);
}