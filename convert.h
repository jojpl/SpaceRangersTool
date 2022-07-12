#pragma once
#include <string_view>
#include "Entities.h"

namespace conv
{
	int to_int(std::string_view sw);
	Entities::GoodsQty unpack_goods_str(std::string_view sw);
	
	template<typename T, typename Ret>
	void parse(Ret T::* field, T* p, 
				std::string_view key, std::string_view value);
}