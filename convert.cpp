#include "convert.h"

namespace conv
{
	int extractId(std::string_view sw)
	{
		// ItemId256
		// [^\d]+(\d+)
		std::string s;
		auto pos = std::find_if_not(sw.crbegin(), sw.crend(), 
			[](char ch){ return isdigit(ch); });

		if (pos != sw.crend() && pos != sw.crbegin())
		{
			size_t offset = std::distance(sw.cbegin(), pos.base());

			int i = 0;
			from_string(i, sw.substr(offset));
			return i;
		}
		else
			throw std::logic_error(__FUNCTION__ " err!");
	}

	void from_string(Entities::GoodsPack& packed, std::string_view sw)
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
