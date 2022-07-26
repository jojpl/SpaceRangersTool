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

		throw std::logic_error(__FUNCTION__ " err!");
	}

	void from_string(Entities::GoodsPack& packed, std::string_view sw)
	{
		std::vector<boost::iterator_range<std::string_view::iterator>> splitVec;
		boost::split(splitVec, sw, [](char ch){ return ch==','; });//boost::is_any_of(","));

		using namespace Entities;
		int iter_enum = (int) GoodsEnum {};
		auto iter_range = splitVec.cbegin();
		
		for (;  iter_enum != ENUM_COUNT(GoodsEnum) && iter_range!= splitVec.cend();
				iter_enum++, iter_range++)
		{
			auto rng = *iter_range;
			int res = 0;
			from_string(res, { &*rng.begin(), rng.size() });
			packed[iter_enum] = res;
		}
	}

}
