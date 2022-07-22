#pragma once
#include <iosfwd>

class performance_tracker
{
	std::ostream& os_;
	long long start_  = 0;
	long long finish_ = 0;
public:
	performance_tracker();
	performance_tracker(std::ostream& os);
	~performance_tracker();
};

