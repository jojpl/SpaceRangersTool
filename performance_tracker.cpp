#include "performance_tracker.hpp"

#include <iostream>
#include <chrono>

using namespace std::chrono;

performance_tracker::performance_tracker()
	: performance_tracker(std::cout)
{}

performance_tracker::performance_tracker(std::ostream& os)
	: os_(os)
{
	auto tp = steady_clock::now();
	start_ = duration_cast<milliseconds>(tp.time_since_epoch()).count();
}

performance_tracker::~performance_tracker()
{
	auto tp = steady_clock::now();
	finish_ = duration_cast<milliseconds>(tp.time_since_epoch()).count();
	#if defined _DEBUG
	os_ << (finish_ - start_) << " ms";
	os_	<< std::endl;
	#endif
}
