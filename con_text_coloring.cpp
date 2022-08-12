#include "con_text_coloring.h"
#include <kwsys\Terminal.h>
#include <string>

#include <windows.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <ostream>
#include <sstream>
#include <iterator>
#include <tuple>

#include <boost/assert.hpp>

namespace color
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
	BOOST_ASSERT((tag.tag == exp.tag && tag.tag_prop == exp.tag_prop && tag.tag_content == exp.tag_content));
	exit(0);
}

void PrintColored(const std::string& str)
{
	return;
	/*
	[tag, pos, size] = find_color_tag(str);
	tag_analyze(tag);
	color = convert_to_nearest_color(tag);
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