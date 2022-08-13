#include "con_text_coloring.h"
#include "common_algo.h"

#include <string>

#include <windows.h>
#include <io.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <ostream>
#include <sstream>
#include <iterator>
#include <tuple>
#include <array>

#include <boost/assert.hpp>

namespace colors
{

using std::string_view;
using namespace std::string_view_literals;
using namespace std::string_literals;

struct Tag_pos
{
string_view tag;
string_view tag_prop;
string_view tag_content;
};
// "<color=255,167,84>Корпус \"Молния\" II</color>"
//  |-----------------tag------------------------|
//         |--prop--|
//                   |-------content------|

constexpr std::string_view color_tag_open_begin = "<color=";
constexpr std::string_view color_tag_open_close = ">";
constexpr std::string_view color_tag_close      = "</color>";

Tag_pos
find_color_tag(const std::string& str)
{
	auto sw = std::string_view{str};

	size_t pos = sw.find(color_tag_open_begin);
	if(pos!=sw.npos)
	{
		auto tag_beg = pos;
		pos += color_tag_open_begin.size();
		
		auto prop_beg = pos;
		pos = sw.find(color_tag_open_close, prop_beg);
		if(pos != sw.npos)
		{
			auto prop_end = pos;
			pos += color_tag_open_close.size();

			auto content_beg = pos;
			pos = sw.find(color_tag_close, content_beg);
			if (pos != sw.npos)
			{
				auto content_end = pos;
				pos += color_tag_close.size();

				auto tag_end = pos;

				Tag_pos result;
				result.tag         = sw.substr(tag_beg, tag_end - tag_beg);
				result.tag_prop    = sw.substr(prop_beg, prop_end - prop_beg);
				result.tag_content = sw.substr(content_beg, content_end - content_beg);
				return result;
			}
		}
	}
	return {};
}

void test_find_color_tag()
{
	auto tag_str = "<color=255,167,84>Корпус \"Молния\" II</color>"s;
	auto tag = find_color_tag(tag_str);
	Tag_pos exp = {
		"<color=255,167,84>Корпус \"Молния\" II</color>"sv,
		"255,167,84"sv, 
		"Корпус \"Молния\" II"sv
	};

	BOOST_ASSERT((tag.tag == exp.tag && tag.tag_prop == exp.tag_prop && tag.tag_content == exp.tag_content));
	
	tag_str = "...some begin...<color=some_color>Content</color>...some end..."s;
	tag = find_color_tag(tag_str);
	exp = {
	"<color=some_color>Content</color>"sv,
	"some_color"sv,
	"Content"sv
	};
	BOOST_ASSERT(tag.tag == exp.tag && tag.tag_prop == exp.tag_prop && tag.tag_content == exp.tag_content);
	
	tag_str = "...simple string no tag..."s;
	tag = find_color_tag(tag_str);
	BOOST_ASSERT(tag.tag.empty() && tag.tag_prop.empty() && tag.tag_content.empty());

	tag_str = "...some begin...<color=some_color>Content...tag close missed..."s;
	tag = find_color_tag(tag_str);
	BOOST_ASSERT(tag.tag.empty() && tag.tag_prop.empty() && tag.tag_content.empty());
	//exit(0);
}

void convert_to_nearest_color(RGB& rgb)
{
	// #ifdef WIN32
	rgb.r = (rgb.r > 255 / 2) ? 255 : 0;
	rgb.g = (rgb.g > 255 / 2) ? 255 : 0;
	rgb.b = (rgb.b > 255 / 2) ? 255 : 0;
}

RGB to_RGB(FColor color)
{
	switch (color)
	{
		case colors::FColor::Black:   return { 0,   0,   0   };
		case colors::FColor::Red:     return { 255, 0,   0   };
		case colors::FColor::Green:   return { 0,   255, 0   };
		case colors::FColor::Yellow:  return { 255, 255, 0   };
		case colors::FColor::Blue:    return { 0,   0,   255 };
		case colors::FColor::Magenta: return { 255, 0,   255 };
		case colors::FColor::Cyan:    return { 0,   255, 255 };
		case colors::FColor::White:   return { 255, 255, 255 };
		default:
			throw std::logic_error(__FUNCTION__);
	}
}

RGB tag_analyze(std::string_view tag)
{
	RGB rgb;
	auto vals = common_algo::unpack(tag);
	if (vals.size() == 1)
	{
		auto color_str = vals[0];
		FColor color;
		from_string(color, color_str);
		rgb = to_RGB(color);
	}
	else if(vals.size() == 3)
	{
		common_algo::from_string(rgb.r,  vals[0]);
		common_algo::from_string(rgb.g,  vals[1]);
		common_algo::from_string(rgb.b,  vals[2]);
	}
	else
	{
		throw std::logic_error(__FUNCTION__);
	}
	return rgb;
}

constexpr WORD FOREGROUND_COLOR_MASK = 0xff;

int convert_to_win_terminal_color(RGB rgb)
{
	int color = 0;
	if(rgb.r) color|= FOREGROUND_RED;
	if(rgb.g) color|= FOREGROUND_GREEN;
	if(rgb.b) color|= FOREGROUND_BLUE;
	return color;
}

void Print_WINConsoleColored(int win_color, std::string_view sw)
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO hOutInfo;

	int fd = _fileno(stdout);
	hOut = (HANDLE)_get_osfhandle(fd);
	GetConsoleScreenBufferInfo(hOut, &hOutInfo);
	WORD save_attr = hOutInfo.wAttributes;
	std::cout.flush();
	SetConsoleTextAttribute(hOut, (save_attr &~FOREGROUND_COLOR_MASK) | win_color);
	std::cout << sw;
	std::cout.flush();
	SetConsoleTextAttribute(hOut, save_attr);
}

std::array<string_view, (size_t) FColor::NUM> fColorStrings
{
	"Black",
	"Red",
	"Green",
	"Yellow",
	"Blue",
	"Magenta",
	"Cyan",
	"White",
};

std::string_view to_string(FColor & color)
{
	return fColorStrings.at((size_t) color);
}

void from_string(FColor& color, std::string_view val)
{
	auto f = std::find(cbegin(fColorStrings), cend(fColorStrings), val );
	if(f == cend(fColorStrings))
		throw std::logic_error(__FUNCTION__);

	color = (FColor) std::distance(cbegin(fColorStrings), f);
}

void PrintColored(std::string str)
{
	// TODO Some color tags in str
	//test_find_color_tag();

	//str = "...some begin...<color=Magenta>Content</color>...some end..."s;
	string_view sw = str;

	auto [tag, tag_prop, tag_content] = find_color_tag(str);
	if (tag.empty() || tag_prop.empty() || tag_content.empty())
	{
		std::cout << str;
		return;
	}
	string_view before_tag (sw.data(), std::distance(sw.data(), tag.data()));
	string_view after_tag (tag.data() + tag.size());

	std::cout << before_tag;

	RGB color = tag_analyze(tag_prop);
	convert_to_nearest_color(color);
	int win_color = convert_to_win_terminal_color(color);
	Print_WINConsoleColored(win_color, tag_content);

	std::cout << after_tag;

	/*
	cut_from_stream([tag, pos, size]);
	forward_to_cout();
	*/
}

}
#if 0
	//kwsysTerminal_cfprintf(color,
	//	stdout, "Hello %s!", "World");
// <color=255,167,84>Корпус "Молния" II</color>
void foo()
{
	std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
	std::ostringstream strCout;
	std::cout.rdbuf(strCout.rdbuf());

	// This goes to the string stream.
	std::cout << std::string(10'000, 'a') << std::endl;

	// Restore old cout.
	std::cout.rdbuf(oldCoutStreamBuf);
	int sz = strCout.str().size();

	// Will output our Hello World! from above.
	std::cout << strCout.str();
}
#endif