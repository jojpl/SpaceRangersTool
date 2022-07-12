#include "convert.h"

#include <charconv>

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

	// default
	template<typename Ret, typename SFINAE = void>
	struct parse_type
	{
		Ret operator()(std::string_view value)
		{
			return value;
		}
	};

	// for int, double
	template<typename Ret>
	struct parse_type<Ret, std::enable_if_t<
		std::is_same_v<Ret, int>
		|| std::is_same_v<Ret, double>>
		>
	{
		Ret operator()(std::string_view value)
		{
			Ret i;
			std::from_chars(value.data(), value.data() + value.size(), i);
			return i;
		}
	};

	template<typename T, typename Ret>
	void parse(Ret T::* field, T* p,
		std::string_view key,
		std::string_view value)
	{
		const auto key_expected = Entities::kv::get_value(field);
		if (key != key_expected)
			return;
		Ret& g = p->*field;
		g = parse_type<Ret>()(value);
	}
}
