#pragma once
#include "Entities.h"

namespace model
{
	using namespace Entities;
	using namespace std::string_literals;
	
	namespace enums
	{
		template<typename T>
		std::array<std::string_view, ENUM_COUNT(T)> init();

		template<typename T>
		const static std::array<std::string_view, ENUM_COUNT(T)> storage = init<T>();

		template<typename T>
		auto& get_storage()
		{
			static_assert(!std::is_const_v<T>, "incorrect to create consts obj!");
			return storage<T>;
		}

		template<typename T>
		static std::vector<std::string_view> init_strings()
		{
			const auto& storage = get_storage<T>();
			std::vector<std::string_view> vsw(storage.cbegin(), storage.cend());
			return vsw;
		}

		template<typename T>
		const auto& get_strings()
		{
			static std::vector<std::string_view> vs = init_strings<T>();
			return vs;
		}

		template<typename T>
		static std::vector<T> init_enums()
		{
			const auto& storage = get_storage<T>();
			std::vector<T> vsw;
			for (size_t i = 0; i < storage.size(); i++)
				vsw.push_back(T(i));

			return vsw;
		}

		template<typename T>
		auto& get_enums()
		{
			static std::vector<T> ve = init_enums<T>();
			return ve;
		}

		template<typename T>
		void from_string(T& field, std::string_view sw)
		{
			const auto& storage = get_storage<T>();
			for (size_t i = 0; i < storage.size(); i++)
			{
				if (storage[i] == sw)
				{
					field = (T)i;
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
			return storage.at((size_t)field);

			throw std::logic_error(
				"empty value for: "s + typeid(field).name()
			);
		}
	}
}