#include "common_algo.h"

#include <boost/algorithm/string/predicate.hpp>

namespace common_algo
{
	size_t soft_search(std::string_view in, std::vector<std::string>& list)
	{
		std::vector<int> mask(list.size(), 0);
		const size_t mask_size = list.size();
		const size_t test_size = in.size();
		boost::is_iequal ieq;

		bool bOnce = true;
		for (size_t i = 0; i < test_size; i++)
		{
			size_t cnt = 0;
			size_t saved_pos = 0;
			for (size_t j = 0; j < mask_size; j++)
			{
				if (!bOnce && !mask[j])
					continue;

				const auto& s = list[j];
				if ((i < s.size()) && ieq(s[i], in[i]))
				{
					mask[j] = 1;

					cnt++;
					saved_pos = j;
				}
				else
					mask[j] = 0;
			}

			bOnce = false;

			if (cnt == 1)
			{
				auto res = list[saved_pos];
				if (boost::istarts_with(res, in))
					return saved_pos;

				return size_t(-1);
			}
			else if (cnt == 0)
				return size_t(-1);
		}
		return size_t(-1);
	}
}
