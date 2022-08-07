#include "convert.h"

#include <boost/algorithm/string/split.hpp>
#include <array>
#include <cctype>

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

	void from_string(Entities::GoodsPack& packed, std::string_view sw)
	{
		auto un = unpack(sw);
		const auto& enums = model::enums::get_enums<Entities::GoodsEnum>();
		if(un.size()!= packed.size()) throw std::logic_error(__FUNCTION__);

		auto iter_enum = enums.cbegin();
		for (auto g_sw: un)
		{
			int res = 0;
			from_string(res, g_sw);

			packed[(int)*iter_enum++] = res;
		}
	}

}
