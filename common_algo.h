#pragma once
#include <string_view>
#include <vector>

namespace common_algo
{

size_t soft_search(std::string_view in, const std::vector<std::string_view>& list);

std::vector<std::string_view> unpack(std::string_view sw);

void from_string(int& ret, std::string_view value);

}

