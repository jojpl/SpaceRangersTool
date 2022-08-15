#pragma once
#include <string>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <optional>

namespace Entities
{
	using std::string;
	using std::vector;
	using std::optional;

	enum class Type
	{
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
		Satellite,
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
		W18,
		W19,
		W20,
		W21,
		W22,
		W23, 
		// terrible enum vals W.. to think about fix

		WB,
		Warrior,

		NUM,
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

		NUM
	};

	enum class OwnersGroup
	{
		Klings,
		Normals,
		Pirates,

		NUM
	};

	enum class Owner
	{
		None,

		Kling,

		Fei,
		Gaal,
		Maloc,
		Peleng,
		People,

		PirateClan,

		NUM
	};

	enum class Goverment
	{
		Anarchy,
		Democracy,
		Dictatorship,
		Monarchy,
		Republic,

		NUM
	};

	struct Item;
	struct HiddenItem;
	struct Star;
	struct Planet;
	struct Ship;
	struct Station;
	struct Player;
	struct Hole;


	struct Unknown
	{};

	#define ENUM_COUNT(Enum) ((size_t) Enum::NUM)

	using GoodsPack = std::array<int, ENUM_COUNT(GoodsEnum)>;

	// help struct, for make structs more plain
	struct Location
	{
		Star*     star = nullptr;
		Planet* planet = nullptr;
	};

	struct GoodsQty
	{
		GoodsPack packed {};
	};

	struct GoodsPrice
	{
		GoodsPack packed {};
	};

	//help struct for calc profits
	struct ObjPrices
	{
		GoodsQty   qty;
		GoodsPrice buy;
		GoodsPrice sale;

		Location location;
		Station* shipshop = nullptr;
	};

	struct EqList
	{
		vector<Item*> list;
	};

	struct ArtsList
	{
		vector<Item*> list;
	};

	struct StarList
	{
		vector<Star*> list;
	};

	struct Junk
	{
		vector<Item*> list;
	};

	struct EqShop
	{
		vector<Item*> list;
	};

	struct Treasure
	{
		vector<HiddenItem*> list;
	};

	struct PlanetList
	{
		vector<Planet*> list;
	};

	struct ShipList
	{
		vector<Ship*> list;
	};

	struct HoleList
	{
		vector<Hole*> list;
	};

	// fake root node - container of all game entities
	struct Global
	{
		int IDay = 0;
		Player* Player = nullptr;
		StarList StarList;
		HoleList HoleList;
	};

	struct Ship
	{
		Ship(int Id_, Location location_)
			: Id(Id_), location(location_)
		{	}

		int Id = 0;	
		string IFullName;
		Type IType {};
		string Name;
		string IPlanet;
		GoodsQty Goods;
		int Money = 0;
		EqList EqList;
		ArtsList ArtsList;

		Location location;
	};

	struct Player : Ship
	{
		Player(int Id_, Location location_)
			: Ship(Id_, location_)
		{	}
		using Inherit = Ship;

		int ICurStarId = 0;
		int Debt = 0;
	};

	struct Warrior : Ship
	{
		Warrior(int Id_, Location location_)
			: Ship(Id_, location_)
		{	}
		using Inherit = Ship;
	};

	struct Station : Ship
	{
		Station(int Id_, Location location_)
			: Ship(Id_, location_)
		{	}
		using Inherit = Ship;

		inline static std::array<Type, 7> allowedTypes
		{ Type::BK, Type::CB, Type::MC, Type::PB, Type::RC, Type::SB, Type::WB };

		EqShop     EqShop;
		GoodsQty   ShopGoods; // base ship dublicate
		GoodsPrice ShopGoodsSale;
		GoodsPrice ShopGoodsBuy;
	};

	struct Item
	{
		Item(int Id_, Location location_)
			: Id(Id_),  location(location_)
		{	}

		int Id = 0;
		string IName;
		Type IType {};
		int	Size = 0;

		optional<int> Ammo;
		optional<int> Armor;
		optional<int> Bonus;
		bool Broken = false;
		optional<bool> BuiltByPirate;
		optional<int> Capacity;
		int	Cost = 0;
		string DomSeries;
		double Durability = 0.0;
		optional<int> Fuel;
		optional<string> IBonusName;
		optional<string> ISeriesName;
		optional<string> ISpecialName;
		optional<int> Jump;
		optional<int> MaxAmmo;
		optional<int> MaxDamage;
		optional<int> MinDamage;
		bool NoDrop = false;
		Owner Owner;
		optional<int> Power;
		optional<int> Radius;
		optional<int> Repair;
		int	Series = 0;
		optional<int> ShipType;
		int	Special = 0;
		optional<int>	Speed;
		optional<int>	SpeedMax;
		optional<int>	SpeedMin;
		optional<string> SysName;
		int	TechLevel = 0;
		optional<double> X;
		optional<double> Y;

		Location location;
	};

	struct HiddenItem
	{
		int LandType = 0;
		int	Depth = 0;
		Item* item = nullptr;
	};

	struct Planet
	{
		Planet(int Id_, Location location_)
			: Id(Id_), location(location_)
		{	}

		int Id = 0;
		string PlanetName;
		Owner Owner;
		string Race;
		string Economy;
		Goverment Goverment;
		int	ISize = 0;
		double OrbitRadius = 0.0;
		double	OrbitAngle = 0.0;
		int	RelationToPlayer = 0;
		int	IMainTechLevel = 0;
		//	TechLevels = 2, 2, 2, 1, 2, 3, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		int	CurrentInvention = 0;
		int	CurrentInventionPoints = 0;

		EqShop     EqShop;
		GoodsQty   ShopGoods;
		GoodsPrice ShopGoodsSale;
		GoodsPrice ShopGoodsBuy;
		Treasure   Treasure;

		Location location;
	};

	struct Star
	{
		Star(int Id_)
			: Id(Id_)
		{	}

		int Id = 0;
		string StarName;
		double X = 0.0;
		double Y = 0.0;
		OwnersGroup Owners;
		ShipList ShipList;
		PlanetList PlanetList;
		Junk Junk;
	};

	struct Hole
	{
		Hole(int Id_)
			: Id(Id_)
		{	}

		int Id           = 0;
		int Star1Id      = 0;
		int Star2Id      = 0;
		int TurnsToClose = 0;

		Location from;
		Location to;
	};
}