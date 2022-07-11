#pragma once
#include <string>
#include <vector>
#include <array>

namespace Entities
{
	using string = std::string;
	using std::vector;
	using std::array;

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


	struct GoodsQty
	{
		array<int, (int)GoodsEnum::GoodsEnum_NUM> packed;
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
		string IName;
	};

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

	struct Global
	{
		int IDay;
		Player* Player;
		StarList StarList;
	};
}