#pragma once
#include "model.hpp"
#include "common_algo.h"

#include <string_view>
#include <charconv>
#include <typeinfo>
#include <limits>
#include <boost/lexical_cast.hpp>

namespace conv
{
	using namespace std::string_literals;

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
	using common_algo::from_string;

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
		ret = std::string{value};
	}

	template<typename T,
		typename SFINAE = std::enable_if_t<std::is_enum_v<T>>
	>
	void from_string(T& ret, std::string_view value)
	{
		model::enums::from_string(ret, value);
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
			ret = std::move(t);
		}
	}

	/* from_string section end */

	
	
	/* to_string section begin */

	template<typename T, 
		typename SFINAE = std::enable_if_t<std::is_enum_v<T>>
		>
	std::string_view to_string(const T& t)
	{
		return model::enums::to_string(t);
	}

	/* to_string section end */
}