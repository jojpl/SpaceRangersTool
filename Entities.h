#pragma once
#include <string>
#include <vector>
#include <array>
#include <map>
#include <string_view>

namespace Entities
{
	using std::string;
	using std::vector;

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
		int IDay;
		Player* Player;
		StarList StarList;
	};

	struct Ship
	{
		int Id;	
		int ICurStarId;
		string IFullName;
		Type IType;
		string Name;
		string IPlanet;
		GoodsQty Goods;
		int Money;
		EqList EqList;
		ArtsList ArtsList;
	};

	struct Player : Ship
	{
		int Debt;
	};

	struct Warrior : Ship
	{

	};

	struct Item
	{
		Item(int Id_)
			: Id(Id_)
		{}

		int Id;
		string IName;
		Type IType;
		int	Size;

		int Ammo;
		int Armor;
		int Bonus;
		bool Broken;
		bool BuiltByPirate;
		int	Capacity;
		int	Cost;
		string DomSeries;
		double Durability;
		int	Fuel;
		string IBonusName;
		string ISeriesName;
		string ISpecialName;
		int	Jump;
		int	MaxAmmo;
		int	MaxDamage;
		int	MinDamage;
		bool NoDrop;
		string Owner;
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
		double X;
		double Y;
	};

	struct HiddenItem
	{
		int LandType;
		int	Depth;
		Item* item;
	};

	struct Planet
	{
		Planet(int Id_)
			:Id(Id_)
		{
		}

		int Id;
		string PlanetName;
		string Owner;
		string Race;
		string Economy;
		string Goverment;
		int	ISize;
		double OrbitRadius;
		double	OrbitAngle;
		int	RelationToPlayer;
		int	IMainTechLevel;
		//	TechLevels = 2, 2, 2, 1, 2, 3, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		int	CurrentInvention;
		int	CurrentInventionPoints;

		EqShop EqShop;
		GoodsQty ShopGoods;
		GoodsPrice ShopGoodsSale;
		GoodsPrice ShopGoodsBuy;
		Treasure Treasure;
	};

	struct Star
	{
		int Id;
		string StarName;
		double X;
		double Y;
		string Owners;
		ShipList ShipList;
		PlanetList PlanetList;
		Junk Junk;
	};
}