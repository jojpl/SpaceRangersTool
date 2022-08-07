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
	date_facet *df = new date_facet{ "%e %b %Y" };
	std::stringstream ss;
	ss.imbue(std::locale{ get_ru_1251_locale(), df });

	date d(3000, 01, 01);
	days day(days_left);
	d = d + day;

	ss << "Game date is:" << d;

	return ss.str();
}

std::locale get_ru_1251_locale()
{
	std::locale loc("rus_rus.1251");
	return loc;
}

}
