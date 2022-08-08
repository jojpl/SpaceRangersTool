#pragma once
#include "Entities.h"
#include <algorithm>
#include <boost/container/stable_vector.hpp>
namespace storage
{

using namespace Entities;

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
	static_assert(!std::is_const_v<T>, "incorrect to create consts obj!");
	static boost::container::stable_vector<T> storage{};
	static storageRegistrator registrator(storage);
	return storage;
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

	template<typename FindPred>
	static T* find(FindPred&& pred)
	{
		auto& storage = storage::get<T>();
		auto f = std::find_if(storage.begin(), storage.end(), 
			std::forward<FindPred>(pred));

		if(f == storage.end()) return nullptr;

		return std::addressof(*f);
	}
};

inline Star*
find_star_by_name(std::string_view sw)
{
	return Factory<Star>::find(
		[sw](const Star& s) {
			return s.StarName == sw;
		}
	);
}

inline Star*
find_star_by_id(int id)
{
	return Factory<Star>::find(
		[id](const Star& s) {
			return s.Id == id;
		}
	);
}

inline Planet*
find_planet_by_name(std::string_view sw)
{
	return Factory<Planet>::find(
		[sw](const Planet& s) {
			return s.PlanetName == sw;
		}
	);
}

inline Location
find_player_cur_location()
{
	auto* player = Factory<Player>::find(
		[](const Player& s) {
			return true;
		}
	);
	return player->location;
}

inline Star*
find_player_cur_star()
{
	auto loc = find_player_cur_location();
	return loc.star;
}

} // namespace storage
