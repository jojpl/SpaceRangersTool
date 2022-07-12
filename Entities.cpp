#include "Entities.h"
#include "convert.h"

#include <algorithm>
#include <map>
#include <string_view>

#include <cctype>

namespace Entities
{
	#define NAME2TYPE_ELEMENT(name) {#name, Type::name}
	const std::map< std::string_view, Type> map
	{
		NAME2TYPE_ELEMENT(Alcohol),
		NAME2TYPE_ELEMENT(Arms),
		NAME2TYPE_ELEMENT(ArtAnalyzer),
		NAME2TYPE_ELEMENT(ArtAntigrav),
		NAME2TYPE_ELEMENT(ArtDecelerate),
		NAME2TYPE_ELEMENT(ArtDefToArms1),
		NAME2TYPE_ELEMENT(ArtDefToArms2),
		NAME2TYPE_ELEMENT(ArtDefToEnergy),
		NAME2TYPE_ELEMENT(ArtDroid),
		NAME2TYPE_ELEMENT(ArtEnergyDef),
		NAME2TYPE_ELEMENT(ArtForsage),
		NAME2TYPE_ELEMENT(ArtFuel),
		NAME2TYPE_ELEMENT(ArtGiperJump),
		NAME2TYPE_ELEMENT(ArtHook),
		NAME2TYPE_ELEMENT(ArtHull),
		NAME2TYPE_ELEMENT(ArtMiniExpl),
		NAME2TYPE_ELEMENT(ArtNano),
		NAME2TYPE_ELEMENT(ArtPDTurret),
		NAME2TYPE_ELEMENT(ArtPower),
		NAME2TYPE_ELEMENT(ArtRadar),
		NAME2TYPE_ELEMENT(ArtScaner),
		NAME2TYPE_ELEMENT(ArtSpeed),
		NAME2TYPE_ELEMENT(ArtSplinter),
		NAME2TYPE_ELEMENT(ArtTransmitter),
		NAME2TYPE_ELEMENT(ArtWeaponToSpeed),
		NAME2TYPE_ELEMENT(BK),
		NAME2TYPE_ELEMENT(CB),
		NAME2TYPE_ELEMENT(CargoHook),
		NAME2TYPE_ELEMENT(Cistern),
		NAME2TYPE_ELEMENT(DefGenerator),
		NAME2TYPE_ELEMENT(Engine),
		NAME2TYPE_ELEMENT(Food),
		NAME2TYPE_ELEMENT(FuelTanks),
		NAME2TYPE_ELEMENT(Hull),
		NAME2TYPE_ELEMENT(Kling),
		NAME2TYPE_ELEMENT(Luxury),
		NAME2TYPE_ELEMENT(MC),
		NAME2TYPE_ELEMENT(Medicine),
		NAME2TYPE_ELEMENT(Minerals),
		NAME2TYPE_ELEMENT(Narcotics),
		NAME2TYPE_ELEMENT(Nod),
		NAME2TYPE_ELEMENT(PB),
		NAME2TYPE_ELEMENT(Pirate),
		NAME2TYPE_ELEMENT(Protoplasm),
		NAME2TYPE_ELEMENT(RC),
		NAME2TYPE_ELEMENT(Radar),
		NAME2TYPE_ELEMENT(Ranger),
		NAME2TYPE_ELEMENT(RepairRobot),
		NAME2TYPE_ELEMENT(SB),
		NAME2TYPE_ELEMENT(Scaner),
		NAME2TYPE_ELEMENT(Technics),
		NAME2TYPE_ELEMENT(Transport),
		NAME2TYPE_ELEMENT(UselessItem),
		NAME2TYPE_ELEMENT(W01),
		NAME2TYPE_ELEMENT(W02),
		NAME2TYPE_ELEMENT(W03),
		NAME2TYPE_ELEMENT(W04),
		NAME2TYPE_ELEMENT(W05),
		NAME2TYPE_ELEMENT(W06),
		NAME2TYPE_ELEMENT(W07),
		NAME2TYPE_ELEMENT(W08),
		NAME2TYPE_ELEMENT(W09),
		NAME2TYPE_ELEMENT(W10),
		NAME2TYPE_ELEMENT(W11),
		NAME2TYPE_ELEMENT(W12),
		NAME2TYPE_ELEMENT(W13),
		NAME2TYPE_ELEMENT(W14),
		NAME2TYPE_ELEMENT(W15),
		NAME2TYPE_ELEMENT(W17),
		NAME2TYPE_ELEMENT(WB),
		NAME2TYPE_ELEMENT(Warrior),
	};
	#undef NAME2TYPE_ELEMENT

	template<>
	Type str2type(std::string_view sw)
	{
		return map.at(sw);
	}

	namespace kv
	{
		// store keys for struct fields
		template<typename T, typename Ret>
		auto& get_storage(Ret T::* field)
		{
			static std::vector<std::pair<Ret T::*, std::string_view>> storage;
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
			//constexpr auto r = offsetof(T, field);
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

		template<typename T, typename Ret>
		std::string_view get_value(S, const Ret T::* field)
		{
			return get_value(field);
		}

		void init_storage_impl();
		
		bool is_inited = false;
		void init_storage()
		{
			if (!is_inited)
			{
				init_storage_impl();
				is_inited = true;
			}
		}

		void init_storage_impl()
		{
			#define BEGIN_DEF_FOR(struct_name) { using t = struct_name;
			#define ADD_DEF(field) add_definition(&t::field, #field);
			#define ADD_DEF_SPECIFIC(field, key) add_definition(&t::field, #key);
			#define END_DEF() }
			//add_definition(&Global::IDay, "IDay");

			BEGIN_DEF_FOR(Global)
				ADD_DEF(IDay)
			END_DEF()

			BEGIN_DEF_FOR(Player)
				ADD_DEF(ICurStarId)
				ADD_DEF(IFullName)
				ADD_DEF(IType);
				ADD_DEF(Name)
				ADD_DEF(IPlanet)
				ADD_DEF(Money);
			END_DEF()

			BEGIN_DEF_FOR(Item)
				ADD_DEF(IName)
				ADD_DEF(IType)
				ADD_DEF(Owner)
				ADD_DEF(Size)
				ADD_DEF(Cost)
				ADD_DEF(NoDrop)
				ADD_DEF(Special)
				ADD_DEF(ISpecialName)
				ADD_DEF(DomSeries)
				ADD_DEF(TechLevel)
				ADD_DEF(Armor)
				ADD_DEF(ShipType)
				ADD_DEF(Series)
				ADD_DEF(ISeriesName)
				ADD_DEF(BuiltByPirate)
			END_DEF()

			#undef BEGIN_DEF_FOR
			#undef ADD_DEF
			#undef ADD_DEF_SPECIFIC
			#undef END_DEF
		}
	}
}