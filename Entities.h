#pragma once
#include <string>
#include <vector>
#include <array>
#include <map>
#include <optional>

namespace Entities
{
	using std::string;
	using std::vector;
	using std::optional;

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
	};
	

	struct Item;
	struct HiddenItem;
	struct Star;
	struct Planet;
	struct Ship;
	struct Player;


	struct Unknown
	{};

	using GoodsPack = std::map<GoodsEnum, int>;

	struct GoodsQty
	{
		GoodsPack packed;
	};

	struct GoodsPrice
	{
		GoodsPack packed;
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

	struct Global
	{
		int IDay = 0;
		Player* Player = nullptr;
		StarList StarList;
	};

	struct Ship
	{
		int Id = 0;	
		int ICurStarId = 0;
		string IFullName;
		Type IType = Type::Unknown;
		string Name;
		string IPlanet;
		GoodsQty Goods;
		int Money = 0;
		EqList EqList;
		ArtsList ArtsList;
	};

	struct Player : Ship
	{
		int Debt = 0;
	};

	struct Warrior : Ship
	{

	};

	struct Item
	{
		Item(int Id_)
			: Id(Id_)
		{}

		int Id = 0;
		string IName;
		Type IType = Type::Unknown;
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
		string IBonusName;
		string ISeriesName;
		string ISpecialName;
		optional<int> Jump;
		optional<int> MaxAmmo;
		optional<int> MaxDamage;
		optional<int> MinDamage;
		bool NoDrop = false;
		string Owner;
		optional<int> Power;
		optional<int> Radius;
		optional<int> Repair;
		int	Series = 0;
		optional<int> ShipType;
		int	Special = 0;
		optional<int>	Speed;
		optional<int>	SpeedMax;
		optional<int>	SpeedMin;
		string SysName;
		int	TechLevel = 0;
		optional<double> X;
		optional<double> Y;
	};

	struct HiddenItem
	{
		int LandType = 0;
		int	Depth = 0;
		Item* item = nullptr;
	};

	struct Planet
	{
		Planet(int Id_)
			:Id(Id_)
		{
		}

		int Id = 0;
		string PlanetName;
		string Owner;
		string Race;
		string Economy;
		string Goverment;
		int	ISize = 0;
		double OrbitRadius = 0.0;
		double	OrbitAngle = 0.0;
		int	RelationToPlayer = 0;
		int	IMainTechLevel = 0;
		//	TechLevels = 2, 2, 2, 1, 2, 3, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		int	CurrentInvention = 0;
		int	CurrentInventionPoints = 0;

		EqShop EqShop;
		GoodsQty ShopGoods;
		GoodsPrice ShopGoodsSale;
		GoodsPrice ShopGoodsBuy;
		Treasure Treasure;
	};

	struct Star
	{
		int Id = 0;
		string StarName;
		double X = 0.0;
		double Y = 0.0;
		string Owners;
		ShipList ShipList;
		PlanetList PlanetList;
		Junk Junk;
	};
}