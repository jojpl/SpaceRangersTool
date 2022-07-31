#pragma once
#include "Entities.h"

namespace model
{
	using namespace std::string_literals;

	template<typename T>
	auto& get_map()
	{
		static std::map< std::string_view, T> map_for;
		return map_for;
	}

	namespace kv
	{
		// restore key for specified struct field
		//template<typename T, typename Ret>
		//std::string_view get_value(const Ret T::* field);

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
		std::string_view get_value(const Ret T::* field)
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