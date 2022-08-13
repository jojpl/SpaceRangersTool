#include "common_algo.h"
#include "datetime.h"

#include <charconv>
#include <exception>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

namespace common_algo
{
using namespace std::placeholders;

size_t soft_search(std::string_view test, const std::vector<std::string_view>& list)
{
	std::locale loc = datetime::try_get_ru_1251_locale();
	boost::is_iequal ieq(loc);
	for (size_t i = 0; i < list.size(); i++)
	{
		if (boost::equals(list[i], test, ieq))
			return i;
	}

	std::vector<char> mask(list.size(), 1);

	size_t cnt = 0, saved_pos = 0;
	for (size_t i = 0; i < test.size(); i++)
	{
		cnt = 0;
		saved_pos = 0;
		for (size_t j = 0; j < mask.size(); j++)
		{
			if (!mask[j])
				continue;

			const auto& s = list[j];
			if (i < s.size())
			{
				if (ieq(s[i], test[i]))
				{
					cnt++;
					saved_pos = j;
					continue;
				}
			}
			mask[j] = 0;
		}
	}
	if (cnt == 1) return saved_pos;
	return size_t(-1);
}

std::vector<std::string_view> unpack(std::string_view sw)
{
	using sw_iters_range = boost::iterator_range<std::string_view::iterator>;

	std::vector<sw_iters_range> splitVec;
	boost::split(splitVec, sw, std::bind(std::equal_to<char>(), _1, ','));

	std::vector<std::string_view> vsw;
	for (auto iter_range : splitVec)
	{
		const char* sw_b = &*iter_range.begin();
		vsw.emplace_back(sw_b, iter_range.size());
	}
	return vsw;
}

void from_string(int& ret, std::string_view value)
{
	std::from_chars_result err =
		std::from_chars(value.data(), value.data() + value.size(), ret);

	if (err.ec != std::errc{})
		throw std::logic_error(__FUNCTION__);
}

} //namespace common_algo
