#pragma once
#include "model.hpp"

#include <string_view>
#include <charconv>
#include <typeinfo>

namespace conv
{
	using namespace std::string_literals;

	int to_int(std::string_view sw);

	int extractId(std::string_view sw);

	void unpack_goods_str(Entities::GoodsPack&, std::string_view sw);
	
	// default
	template<typename Ret, typename SFINAE = void>
	struct parse_type {};

	template<typename Ret>
	struct parse_type<Ret, std::enable_if_t<
		   std::is_same_v<Ret, Entities::GoodsQty>
		|| std::is_same_v<Ret, Entities::GoodsPrice>>
		>
	{
		static Ret from_string(std::string_view value)
		{
			Ret goods{};

			unpack_goods_str(goods.packed, value);
			return goods;
		}
	};

	// for int, double
	template<typename Ret>
	struct parse_type<Ret, std::enable_if_t<
		   std::is_same_v<Ret, int>
		|| std::is_same_v<Ret, double>>
		>
	{
		static Ret from_string(std::string_view value)
		{
			Ret i;
			std::from_chars_result err =
				std::from_chars(value.data(), value.data() + value.size(), i);

			if (err.ec != std::errc{})
				throw std::logic_error(__FUNCTION__ " err!");
			return i;
		}
	};

	template<>
	struct parse_type<std::string>
	{
		static std::string from_string(std::string_view value)
		{
			return {value.data(), value.data() + value.size()};
		}
	};

	template<>
	struct parse_type<Entities::Type>
	{
		static Entities::Type from_string(std::string_view value)
		{
			return model::converter<Entities::Type>().from_string(value);
		}
	};

	template<typename Ret>
	struct parse_type<std::optional<Ret>>
	{
		static std::optional<Ret> from_string(std::string_view value)
		{
			return { parse_type<Ret>::from_string(value) };
		}
	};
	
	template<>
	struct parse_type<bool>
	{
		static bool from_string(std::string_view value)
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

	template<typename T, typename Ret, typename Base,
		typename SFINAE = std::enable_if_t<std::is_base_of_v<T, Base>>
		>
	bool parse(Ret T::* field, Base* p,
				std::string_view key,
				std::string_view value)
	{
		const auto key_expected = model::kv::get_value(field);
		if (key != key_expected)
			return false;

		p->*field = parse_type<Ret>::from_string(value);
		return true;
	}
}