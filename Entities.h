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
		W16,
		W17,
		WB,
		Warrior,

		Type_NUM,
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
	
	

	template<typename T>
	auto& get_map()
	{
		static std::map< std::string_view, T> map_for;
		return map_for;
	}

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
		int	Size;				// = 625

		int Ammo;
		int Armor;
		int Bonus;
		bool Broken;
		bool BuiltByPirate;
		int	Capacity;
		int	Cost;
		string	DomSeries;
		double	Durability;
		int	Fuel;
		string IBonusName;
		string ISeriesName;
		string ISpecialName;
		int	Jump;
		int	MaxAmmo;
		int	MaxDamage;
		int	MinDamage;
		bool	NoDrop;
		string	Owner;
		int	Power;
		int	Radius;
		int	Repair;
		int	Series;
		int	ShipType;
		int	Special;
		int	Speed;
		int	SpeedMax;
		int	SpeedMin;
		int	SysName;
		int	TechLevel;
		double	X;
		double	Y;
	};

	struct StarList
	{
		vector<Star*> list;
	};

	struct Planet
	{

	};

	struct PlanetList
	{
		vector<Planet*> list;
	};

	struct ShipList
	{
		vector<Ship*> list;
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

	struct Global
	{
		int IDay;
		Player* Player;
		StarList StarList;
	};

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