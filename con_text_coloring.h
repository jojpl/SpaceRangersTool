#pragma once

#include <string>

namespace colors
{

struct RGB
{
	int r = 0;
	int g = 0;
	int b = 0;
};

enum class FColor
{
	/* Foreground Color */
	Black = 0,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	White,

	NUM
};

std::string_view to_string(FColor& color);

void from_string(FColor& color, std::string_view val);

void PrintColored(std::string str);

};

