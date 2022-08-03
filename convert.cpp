#include "convert.h"

#include <boost/algorithm/string/split.hpp>
#include <array>

namespace conv
{
	int extractId(std::string_view sw)
	{
		// ItemId256
		auto pos = std::find_if_not(sw.crbegin(), sw.crend(), 
			[](char ch){ return ::isdigit(ch); });

		if (pos != sw.crend() && pos != sw.crbegin())
		{
			size_t offset = std::distance(sw.cbegin(), pos.base());

			int i = 0;
			from_string(i, sw.substr(offset));
			return i;
		}

		throw std::logic_error("in "s + __FUNCTION__ + " err!");
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
