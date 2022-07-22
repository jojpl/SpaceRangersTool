#pragma once
#include <functional>
#include <string>

class filefinder
{
	using handler_type = void(*)(std::string);
	handler_type handler = nullptr;
public:
	void set_handler(handler_type handler_) {handler = handler_;}
	void find();
};

