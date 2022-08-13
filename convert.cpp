#include "convert.h"
#include "common_algo.h"

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

	void from_string(Entities::GoodsPack& packed, std::string_view sw)
	{
		auto un = common_algo::unpack(sw);
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
