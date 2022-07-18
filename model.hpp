#pragma once

#include "Entities.h"

namespace Entities
{
	template<typename T>
	auto& get_map()
	{
		static std::map< std::string_view, T> map_for;
		return map_for;
	}

	template<typename T>
	struct converter
	{
		T from_string(std::string_view sw)
		{
			const auto& map = get_map<T>();
			return map.at(sw);
		}

		bool is_string_for(std::string_view sw)
		{
			const auto& map = get_map<T>();
			return map.find(sw) != map.cend();
		}
	};

	namespace kv
	{
		void init_storage();

		// restore key for specified struct field
		template<typename T, typename Ret>
		std::string_view get_value(const Ret T::* field);

		template<typename T, typename Ret>
		inline std::vector<std::pair<Ret T::*, std::string_view>> storage{};

		// store keys for struct fields
		template<typename T, typename Ret>
		auto& get_storage(Ret T::* field)
		{
			return storage<T, Ret>;
		}

		template<typename T, typename Ret>
		void add_definition(const Ret T::* field,
			std::string_view key)
		{
			auto& storage = get_storage(field);
			storage.push_back({ field, key });
		}

		template<typename T, typename Ret>
		std::string_view get_value(const Ret T::* field)
		{
			const auto& storage = get_storage(field);
			for (const auto& item : storage)
			{
				if (item.first == field)
					return item.second;
			}

			using namespace std::string_literals;
			throw std::logic_error(
				"empty value for: "s + typeid(field).name()
			);
		}
	}
}