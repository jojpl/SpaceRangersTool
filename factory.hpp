#pragma once
#include "Entities.h"

#include "boost\container\stable_vector.hpp"
namespace storage
{
	//template <typename T>
	//inline static boost::container::stable_vector<T> storage {};

	template <typename T>
	auto& get()
	{
		static boost::container::stable_vector<T> storage{};
		return storage;
	}

}

template <typename T>
class Factory
{
public:

	template<typename ... Args>
	static T* create(Args &&...args)
	{
		auto& storage = storage::get<T>();
		auto* item = &storage.emplace_back(std::forward<Args>(args)...);
		return item;
	}
};
