#include "convert.h"

namespace conv
{
	int extractId(std::string_view sw)
	{
		// ItemId256
		auto pos = sw.find_last_of("Id");
		if(pos == sw.npos || (++pos == sw.size()))
			throw std::logic_error(__FUNCTION__ " err!");
		int id = 0;
		from_string(id, sw);
		return id;
	}

	void unpack_goods_str(Entities::GoodsPack& packed, std::string_view sw)
	{
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

			int res = 0;
			from_string(res, std::string_view(p1, p2 - p1));
			p1 = p2;

			packed[Entities::GoodsEnum(cnt)] = res;
			cnt++;
		}
	}

}
