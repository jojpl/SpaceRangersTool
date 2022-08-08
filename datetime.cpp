#include "datetime.h"

#include <chrono>
#include <ctime>
#include <locale>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace datetime
{
using namespace boost::gregorian;

std::string get_cur_game_date_str(int days_left)
{
	date d(3300, 01, 01);
	std::stringstream ss;
	int real_days_left = days_left - 301;
	if(real_days_left > 0)
	{
		date_facet *df = new date_facet{ "%e %b %Y" };
		ss.imbue(std::locale{ get_ru_1251_locale(), df });

		days day(real_days_left); // 301 - first day
		d = d + day;
	}
	else
		d = date{};

	ss << "Game date is: " << d;

	return ss.str();
}

std::locale get_ru_1251_locale()
{
	std::locale loc("rus_rus.1251");
	return loc;
}

}
