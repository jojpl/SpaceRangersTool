#pragma once
#include "Entities.h"

#include "boost\container\stable_vector.hpp"
namespace storage
{
	//inline static boost::container::stable_vector<T> storage {};

	struct Registrator
	{
		virtual void clear() = 0;
		virtual ~Registrator(){};
		
		inline static std::vector<Registrator*> arr {};
		static void clear_storage()
		{
			for (size_t i = 0; i < arr.size(); i++)
			{
				arr[i]->clear();
			}
		}
	};

	

	template <typename T>
	struct storageRegistrator : Registrator
	{
		storageRegistrator(boost::container::stable_vector<T>& storage)
			: storage_(storage)
		{
			arr.push_back(this);
		}

		void clear() override
		{
			storage_.clear();
		}

		boost::container::stable_vector<T>& storage_;
	};

	template <typename T>
	inline auto& get()
	{
		static boost::container::stable_vector<T> storage{};
		static storageRegistrator registrator(storage);
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
