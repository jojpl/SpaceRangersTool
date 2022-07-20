#include "convert.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
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
		constexpr static const std::array<GoodsEnum, (size_t) GoodsEnum::GoodsEnum_NUM>
			goodsEnum_arr
		{
			GoodsEnum::Food,
			GoodsEnum::Medicine,
			GoodsEnum::Alcohol,
			GoodsEnum::Minerals,
			GoodsEnum::Luxury,
			GoodsEnum::Technics,
			GoodsEnum::Arms,
			GoodsEnum::Narcotics,
		};

		auto goodsEnum_iter = goodsEnum_arr.cbegin();
		auto iter_range = splitVec.cbegin();
		for (; goodsEnum_iter!= goodsEnum_arr.cend() && iter_range!= splitVec.cend();
			goodsEnum_iter++, iter_range++)
		{
			auto rng = *iter_range;
			int res = 0;
			from_string(res, { &*rng.begin(), rng.size() });
			packed[*goodsEnum_iter] = res;
		}
	}

}
