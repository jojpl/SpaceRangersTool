#pragma once
#include "model.hpp"

#include <string_view>
#include <charconv>
#include <typeinfo>
#include <limits>
#include <boost/lexical_cast.hpp>

namespace conv
{
	using namespace std::string_literals;
	using namespace std::placeholders;

	int extractId(std::string_view sw);
	
	/* from_string section begin */

	void from_string(Entities::GoodsPack&, std::string_view sw);

	inline void from_string(Entities::GoodsQty& ret, std::string_view value)
	{
		from_string(ret.packed, value);
	}

	inline void from_string(Entities::GoodsPrice& ret, std::string_view value)
	{
		from_string(ret.packed, value);
	}

	// for int, double
	inline void from_string(int& ret, std::string_view value)
	{
		std::from_chars_result err =
			std::from_chars(value.data(), value.data() + value.size(), ret);

		if (err.ec != std::errc{})
			throw std::logic_error(__FUNCTION__);
	}

	inline void from_string(double& ret, std::string_view value)
	{
#ifdef _MSC_VER
		std::from_chars_result err =
			std::from_chars(value.data(), value.data() + value.size(), ret);

		if (err.ec != std::errc{})
			throw std::logic_error(__FUNCTION__);
#else
		constexpr size_t max_digits = std::numeric_limits<double>::max_digits10;
		static std::string buf (max_digits + 1, '\0');
		buf.assign(value.data(), value.data() + std::min(max_digits, value.size()));
		
		size_t ind = 0;
		ret = std::stod(buf, &ind);
		//buf.clear();
#endif
	}

	inline void from_string(std::string& ret, std::string_view value)
	{
		ret = {value.data(), value.data() + value.size()};
	}

	inline void from_string(Entities::Type& ret, std::string_view value)
	{
		// fix it. has same functionality!
		const auto& map = model::enums::get_map<Entities::Type>();
		ret = map.at(value);
		//model::converter<Entities::Type>::from_string(ret, value);
	}

	inline void from_string(Entities::GoodsEnum& ret, std::string_view value)
	{
		const auto& map = model::enums::get_map<Entities::GoodsEnum>();
		ret = map.at(value);
	}
	
	inline void from_string(bool& ret, std::string_view value)
	{
		if(value == "False")
			ret = false;
		else if(value == "True")
			ret = true;
		else
		{
			throw std::logic_error(__FUNCTION__);
		}
	}

	// insert from_string specializations here =>

	template<typename T>
	inline void from_string(std::optional<T>& ret, std::string_view value)
	{
		if (value.empty())
			ret = {};
		else
		{
			T t {};
			from_string(t, value);
			ret = { t };
		}
	}

	template<typename T, typename Ret, typename Base,
		typename SFINAE = std::enable_if_t<std::is_base_of_v<T, Base>>
		>
	inline bool parse(Ret T::* field, Base* p,
				std::string_view key,
				std::string_view value)
	{
		const auto key_expected = model::kv::get_value(field);
		if (key != key_expected)
			return false;

		from_string(p->*field, value);
		return true;
	}
	/* from_string section end */

	
	
	/* to_string section begin */

	inline std::string_view to_string(const Entities::GoodsEnum& t)
	{
		const auto& map = model::enums::get_map<Entities::GoodsEnum>();
		for (auto&[key, value] : map)
		{
			if (value == t)
				return key;
		}
		throw std::logic_error(__FUNCTION__);
	}

	inline std::string_view to_string(const Entities::Type& t)
	{
		const auto& map = model::enums::get_map<Entities::Type>();
		for (auto&[key, value] : map)
		{
			if (value == t)
				return key;
		}
		throw std::logic_error(__FUNCTION__);
	}
	/* to_string section end */
}