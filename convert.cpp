#include "convert.h"

#include <boost/algorithm/string/split.hpp>
#include <array>
#include <functional>
#include <cctype>
#include <limits>

namespace conv
{
	int extractId(std::string_view sw)
	{
		std::string_view tmp = sw;

		size_t pos = 0;
		for (; !tmp.empty() && std::isdigit(tmp.back()); pos++)
			tmp.remove_suffix(1);

		if(!pos) throw std::logic_error(__FUNCTION__);
		tmp = sw.substr(sw.size() - pos);

		int i = 0;
		from_string(i, tmp);
		return i;
	}

	void from_string(Entities::GoodsPack& packed, std::string_view sw)
	{
		std::vector<boost::iterator_range<std::string_view::iterator>> splitVec;
		boost::split(splitVec, sw, std::bind(std::equal_to<char>(), _1, ','));


		int iter_enum = (int)Entities::GoodsEnum {};
		auto iter_range = splitVec.cbegin();
		
		for (;  iter_enum != ENUM_COUNT(Entities::GoodsEnum) && iter_range!= splitVec.cend();
				iter_enum++, iter_range++)
		{
			auto rng = *iter_range;
			int res = 0;
			from_string(res, { &*rng.begin(), rng.size() });
			packed[iter_enum] = res;
		}
	}

}
