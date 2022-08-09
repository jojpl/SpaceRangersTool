#pragma once
#include <iosfwd>
#include <string>

class performance_tracker
{
	std::ostream& os_;
	std::string mes_;
	long long start_  = 0;
	long long finish_ = 0;
	bool enabled_ = false;
public:
	performance_tracker();
	performance_tracker(std::string mes);
	performance_tracker(std::ostream& os, std::string mes);
	~performance_tracker();
};

