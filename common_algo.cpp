#include "common_algo.h"

#include <boost/algorithm/string/predicate.hpp>

namespace common_algo
{

size_t soft_search(std::string_view test, const std::vector<std::string_view>& list)
{
#ifdef _WIN32
	std::locale cp1251_locale("rus_rus.1251"); //win
#else
	std::locale cp1251_locale("ru_RU.CP1251");
#endif
	boost::is_iequal ieq(cp1251_locale);
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

} //namespace common_algo
