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
	template<typename Ret>
	static void from_string(Ret& ret, std::string_view value)
	{
		static_assert(false, "oups! do specialize " __FUNCTION__);
		auto n = typeid(ret).name();
		int i = 8;
	};

	template<>
	static void from_string(Entities::GoodsQty& ret, std::string_view value)
	{
		unpack_goods_str(ret.packed, value);
	}

	template<>
	static void from_string(Entities::GoodsPrice& ret, std::string_view value)
	{
		unpack_goods_str(ret.packed, value);
	}

	// for int, double
	template<>
	static void from_string(int& ret, std::string_view value)
	{
		std::from_chars_result err =
			std::from_chars(value.data(), value.data() + value.size(), ret);

		if (err.ec != std::errc{})
			throw std::logic_error(__FUNCTION__ " err!");
	}

	template<>
	static void from_string(double& ret, std::string_view value)
	{
		std::from_chars_result err =
			std::from_chars(value.data(), value.data() + value.size(), ret);

		if (err.ec != std::errc{})
			throw std::logic_error(__FUNCTION__ " err!");
	}

	template<>
	static void from_string(std::string& ret, std::string_view value)
	{
		ret = {value.data(), value.data() + value.size()};
	}

	template<>
	static void from_string(Entities::Type& ret, std::string_view value)
	{
		// fix it. has same functionality!
		ret = model::converter<Entities::Type>().from_string(value);
	}

	template<typename T>
	static void from_string(std::optional<T>& ret, std::string_view value)
	{
		if(value.empty()) 
			ret = {};
		else 
		{
			T t;
			from_string(t, value);
			ret = { t };
		}
	}
	
	template<>
	static void from_string(bool& ret, std::string_view value)
	{
		if(value == "False")
			ret = false;
		else if(value == "True")
			ret = true;
		else
		{
			throw std::logic_error(__FUNCTION__ " err!");
		}
	}

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

		from_string(p->*field, value);
		return true;
	}
}