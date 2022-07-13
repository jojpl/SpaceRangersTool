#pragma once
#include <string>
#include <vector>
#include <array>
#include <map>
#include <string_view>

namespace Entities
{
	using string = std::string;
	using std::vector;
	using std::array;
	using std::map;

	enum class Type
	{
		Unknown,

		Alcohol,
		Arms,
		ArtAnalyzer,
		ArtAntigrav,
		ArtDecelerate,
		ArtDefToArms1,
		ArtDefToArms2,
		ArtDefToEnergy,
		ArtDroid,
		ArtEnergyDef,
		ArtForsage,
		ArtFuel,
		ArtGiperJump,
		ArtHook,
		ArtHull,
		ArtMiniExpl,
		ArtNano,
		ArtPDTurret,
		ArtPower,
		ArtRadar,
		ArtScaner,
		ArtSpeed,
		ArtSplinter,
		ArtTransmitter,
		ArtWeaponToSpeed,
		BK,
		CB,
		CargoHook,
		Cistern,
		DefGenerator,
		Engine,
		Food,
		FuelTanks,
		Hull,
		Kling,
		Luxury,
		MC,
		Medicine,
		Minerals,
		Narcotics,
		Nod,
		PB,
		Pirate,
		Protoplasm,
		RC,
		Radar,
		Ranger,
		RepairRobot,
		SB,
		Scaner,
		Technics,
		Transport,
		UselessItem,
		W01,
		W02,
		W03,
		W04,
		W05,
		W06,
		W07,
		W08,
		W09,
		W10,
		W11,
		W12,
		W13,
		W14,
		W15,
		W17,
		WB,
		Warrior,

		Type_NUM,
	};
	
	extern const std::map< std::string_view, Type> map_for_T;
	namespace
	{
		template<typename T>
		auto& get_map()
		{
			return map_for_T;
		}
	}

	template<typename T>
	struct converter
	{
		T from_string(std::string_view sw)
		{
			const auto& map = get_map<T>();
			return map.at(sw);
		}
	};

	enum class GoodsEnum
	{
		Food,
		Medicine,
		Alcohol,
		Minerals,
		Luxury,
		Technics,
		Arms,
		Narcotics,

		GoodsEnum_NUM
	};


	struct Item;
	struct Star;
	struct Planet;
	struct Ship;
	struct Player;


	struct Unknown
	{};

	struct GoodsQty
	{
		map<GoodsEnum, int> packed;
	};

	struct EqList
	{
		vector<Item*> list;
	};

	struct ArtsList
	{
		vector<Item*> list;
	};

	struct Ship
	{

	};

	struct Player
	{
		int ICurStarId;// = 23;
		string IFullName; //= Рейнджер Толоймак
		Type	IType;// = Ranger
		string	Name;// = Толоймак
		string	IPlanet;// = Палтеомео
		GoodsQty Goods;
		int Money;
		EqList EqList;
		ArtsList ArtsList;
	};

	struct Item
	{
		int id;
		string IName;
		Type	IType;			// = Hull
		string	Owner;			// = Maloc
		int	Size;				// = 625
		int	Cost;				//= 10596
		bool NoDrop;			// = False
		//	Durability = 99.9999984306749
		//	Broken = False
		//	Bonus = 0
		int	Special;			// = 202
		string ISpecialName;	// = Корпус "Молния" I
		string DomSeries;		// = Blazer
		int	TechLevel;			// = 1
		int	Armor;				// = 2
		int	ShipType;			// = 9
		int	Series;				// = 78
		string ISeriesName;		// = Серия "Трантболл"
		bool BuiltByPirate;		// = False
	};
#if 0
	struct Planet
	{

	};

	struct StarList
	{
		vector<Star*> list;
	};

	struct ShipList
	{
		vector<Ship*> list;
	};

	struct PlanetList
	{
		vector<Planet*> list;
	};

	struct Star
	{
		int Id;
		string StarName;
		double X;// = 19.0000007326204
		double Y;// = 16.0000004361028
		string Owners;
		ShipList ShipList;
		PlanetList PlanetList;
	};
#endif
	struct Global
	{
		int IDay;
		Player* Player;
		//StarList StarList;
	};
	
	namespace kv
	{
		void init_storage();

		// restore key for specified struct field
		template<typename T, typename Ret>
		std::string_view get_value(const Ret T::* field);

		template<typename T, typename Ret>
		inline std::vector<std::pair<Ret T::*, std::string_view>> storage {};

		// store keys for struct fields
		template<typename T, typename Ret>
		auto& get_storage(Ret T::* field)
		{
			//inline static std::vector<std::pair<Ret T::*, std::string_view>> storage;
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
	}
}