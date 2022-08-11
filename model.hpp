#pragma once
#include "Entities.h"

namespace model
{
	using namespace Entities;
	using namespace std::string_literals;
	
	namespace enums
	{
		template<typename T>
		auto& get_storage()
		{
			static_assert(!std::is_const_v<T>, "incorrect to create consts obj!");
			static std::vector< std::pair<std::string_view, T> > storage;
			return storage;
		}

		template<typename T>
		auto& get_strings()
		{
			static std::vector<std::string_view> vs;
			return vs;
		}

		template<typename T>
		auto& get_enums()
		{
			static std::vector<T> vs;
			return vs;
		}

		template<typename T>
		void add_definition(const T& value,
			std::string_view key)
		{
			auto& storage = get_storage<T>();
			storage.push_back({ key, value });

			auto& enums = get_enums<T>();
			enums.push_back(value);

			auto& strings = get_strings<T>();
			strings.push_back(key);
		}

		template<typename T>
		void from_string(T& field, std::string_view sw)
		{
			const auto& storage = get_storage<T>();
			//auto it = std::find_if(cbegin(storage), cend(storage), 
			//	[sw](const auto& p){
			//		return p.first == sw;
			//	}
			//);
			//if(it!=storage.cend())
			//{
			//	const auto& [key, value] = *it;
			//	field = value;
			//	return;
			//}
			for (const auto&[key, val] : storage)
			{
				if (key == sw)
				{
					field = val;
					return;
				}
			}

			throw std::logic_error(
				"empty value for: "s + typeid(field).name()
			);
		}

		template<typename T>
		std::string_view to_string(const T& field)
		{
			const auto& storage = get_storage<T>();
			for (const auto& [key, val] : storage)
			{
				if (val == field)
					return key;
			}

			throw std::logic_error(
				"empty value for: "s + typeid(field).name()
			);
		}
	}

	namespace kv
	{
		// store keys for struct fields
		template<typename T, typename Ret>
		auto& get_storage()
		{
			// contain definitions for parse Entities::T fields -> string
			static_assert(!std::is_const_v<T>,   "incorrect to create consts obj!");
			static_assert(!std::is_const_v<Ret>, "incorrect to create consts obj!");
			static std::map<std::string_view, const Ret T::*> storage{};
			return storage;
		}

		template<typename T, typename Ret>
		void add_definition(const Ret T::* field,
			std::string_view key)
		{
			auto& storage = get_storage<T, Ret>();
			storage.insert({ key, field });
		}

		template<typename T, typename Ret>
		std::string_view to_string(const Ret T::* field)
		{
			const auto& storage = get_storage<T, Ret>();
			for (const auto& [key, val] : storage)
			{
				if (val == field)
					return key;
			}

			throw std::logic_error(
				"empty value for: "s + typeid(field).name()
			);
		}
	}
}