#pragma once
#include "Entities.h"

namespace model
{
	using namespace std::string_literals;
	
	namespace enums
	{
		template<typename T>
		auto& get_storage()
		{
			static std::map<std::string_view, T> map_for;
			return map_for;
		}

		template<typename T>
		auto& get_strings()
		{
			static std::vector<std::string> vs;
			return vs;
		}

		template<typename T>
		auto& get_enums()
		{
			static std::vector<T> vs;
			return vs;
		}

		template<typename T>
		void add_definition(std::string_view key,
			const T& value)
		{
			auto& storage = get_storage<T>();
			storage.insert({ key, value });

			auto& enums = get_enums<T>();
			enums.push_back(value);

			auto& strings = get_strings<T>();
			strings.push_back(std::string{key});
		}

		template<typename T>
		void from_string(T& field, std::string_view value)
		{
			const auto& storage = get_storage<T>();
			try
			{
				field = storage.at(value);
			}
			catch (const std::exception&)
			{
				throw std::logic_error(
					"empty value for: "s + typeid(field).name()
				);
			}
		}

		template<typename T>
		std::string_view to_string(const T& field)
		{
			const auto& storage = get_storage<T>();
			for (const auto& item : storage)
			{
				if (item.second == field)
					return item.first;
			}

			throw std::logic_error(
				"empty value for: "s + typeid(field).name()
			);
		}
	}

	namespace kv
	{
		// pointer to member don't have operator< and can't be key for map

		// store keys for struct fields
		template<typename T, typename Ret>
		auto& get_storage(Ret T::* field)
		{
			// contain definitions for parse Entities::T fields -> string
			static std::vector<std::pair<Ret T::*, std::string_view>> storage{};
			return storage;
		}

		template<typename T, typename Ret>
		void add_definition(const Ret T::* field,
			std::string_view key)
		{
			auto& storage = get_storage(field);
			storage.push_back({ field, key });
		}

		template<typename T, typename Ret>
		std::string_view to_string(const Ret T::* field)
		{
			const auto& storage = get_storage(field);
			for (const auto& item : storage)
			{
				if (item.first == field)
					return item.second;
			}

			throw std::logic_error(
				"empty value for: "s + typeid(field).name()
			);
		}
	}
}