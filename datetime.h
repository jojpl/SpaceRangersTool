#pragma once
#include <string>
#include <iosfwd>

namespace datetime
{

std::string get_cur_game_date_str(int days_left);
std::locale get_ru_1251_locale();

}