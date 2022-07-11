#include "convert.h"

#include <charconv>

namespace conv
{
	int to_int(std::string_view sw)
	{
		int res;
		std::from_chars_result err =
			std::from_chars(sw.data(), sw.data() + sw.size(), res);

		if (err.ec != std::errc{})
			throw std::logic_error("std::from_chars err");
		return res;
	}
}
