#include "performance_tracker.hpp"
#include "programargs.hpp"

#include <iostream>
#include <chrono>

using namespace std::chrono;

performance_tracker::performance_tracker()
	: performance_tracker(std::cout, "")
{}

performance_tracker::performance_tracker(std::string mes)
	: performance_tracker(std::cout, mes)
{}

performance_tracker::performance_tracker(std::ostream& os, std::string mes)
	: os_(os), mes_(mes), enabled_(options::get_opt().perf)
{
	if(!enabled_) return;

	auto tp = steady_clock::now();
	start_ = duration_cast<milliseconds>(tp.time_since_epoch()).count();
}

performance_tracker::~performance_tracker()
{
	if (!enabled_) return;

	auto tp = steady_clock::now();
	finish_ = duration_cast<milliseconds>(tp.time_since_epoch()).count();
	os_ << (finish_ - start_) << " ms" <<"   " << mes_;
	os_	<< std::endl;
}
