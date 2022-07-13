#pragma once
#include <string_view>
#include "Entities.h"
#include <charconv>
#include <typeinfo>

namespace conv
{
	int to_int(std::string_view sw);

	int extractId(std::string_view sw);

	Entities::GoodsQty unpack_goods_str(std::string_view sw);
	
	// default
	template<typename Ret, typename SFINAE = void>
	struct parse_type
	{
		Ret operator()(std::string_view value)
		{
			static_assert(false, "default using");
			//return value;
		}
	};

	template<>
	struct parse_type<std::string>
	{
		std::string operator()(std::string_view value)
		{
			return {value.data(), value.data() + value.size()};
		}
	};

	template<>
	struct parse_type<Entities::Type>
	{
		Entities::Type operator()(std::string_view value)
		{
			return Entities::converter<Entities::Type>().from_string(value);
		}
	};	
	
	template<>
	struct parse_type<Entities::GoodsQty>
	{
		Entities::GoodsQty operator()(std::string_view value)
		{
			return conv::unpack_goods_str(value);
		}
	};

	template<>
	struct parse_type<bool>
	{
		bool operator()(std::string_view value)
		{
			if(value == "False")
				return false;
			else if(value == "True")
				return true;
			else
			{
				throw std::logic_error(__FUNCTION__ " err!");
			}
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
	bool parse(Ret T::* field, T* p,
		std::string_view key,
		std::string_view value)
	{
		const auto key_expected = Entities::kv::get_value(field);
		if (key != key_expected)
			return false;
		Ret& g = p->*field;
		g = parse_type<Ret>()(value);
		return true;
	}
}