#include "convert.h"

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

	int extractId(std::string_view sw)
	{
		// ItemId256
		auto pos = sw.find_last_of("Id");
		if(pos == sw.npos || (++pos == sw.size()))
			throw std::logic_error(__FUNCTION__ " err!");
			
		return to_int(sw.substr(pos));
	}

	Entities::GoodsQty unpack_goods_str(std::string_view sw)
	{
		Entities::GoodsQty goods;
		const char* beg = sw.data();
		const char* end = sw.data() + sw.size();
		const char* p1 = beg;
		const char* p2 = beg;

		int cnt = 0;
		while (p1 != end)
		{
			while ((p1 != end) && !isdigit(*p1)) p1++;
			p2 = p1;
			while ((p2 != end) && isdigit(*p2)) p2++;

			int res = conv::to_int(std::string_view(p1, p2 - p1));
			p1 = p2;

			goods.packed[Entities::GoodsEnum(cnt)] = res;
			cnt++;
		}
		return goods;
	}

}
