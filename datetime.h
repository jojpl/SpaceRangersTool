#pragma once
#include <string>
#include <iosfwd>
#include <locale>

namespace datetime
{

std::string get_cur_game_date_str(int days_left);
std::locale try_get_ru_1251_locale();

}