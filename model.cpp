#include "model.hpp"

namespace model
{
namespace enums
{
	void init_converter_impl()
	{
		#define BEGIN_FOR(struct_name) { using t = struct_name;
		#define NAME2TYPE_ELEMENT(name) add_definition(t::name, #name);
		#define END_FOR() }

		BEGIN_FOR(Type)
			NAME2TYPE_ELEMENT(Alcohol)
			NAME2TYPE_ELEMENT(Arms)
			NAME2TYPE_ELEMENT(ArtAnalyzer)
			NAME2TYPE_ELEMENT(ArtAntigrav)
			NAME2TYPE_ELEMENT(ArtDecelerate)
			NAME2TYPE_ELEMENT(ArtDefToArms1)
			NAME2TYPE_ELEMENT(ArtDefToArms2)
			NAME2TYPE_ELEMENT(ArtDefToEnergy)
			NAME2TYPE_ELEMENT(ArtDroid)
			NAME2TYPE_ELEMENT(ArtEnergyDef)
			NAME2TYPE_ELEMENT(ArtForsage)
			NAME2TYPE_ELEMENT(ArtFuel)
			NAME2TYPE_ELEMENT(ArtGiperJump)
			NAME2TYPE_ELEMENT(ArtHook)
			NAME2TYPE_ELEMENT(ArtHull)
			NAME2TYPE_ELEMENT(ArtMiniExpl)
			NAME2TYPE_ELEMENT(ArtNano)
			NAME2TYPE_ELEMENT(ArtPDTurret)
			NAME2TYPE_ELEMENT(ArtPower)
			NAME2TYPE_ELEMENT(ArtRadar)
			NAME2TYPE_ELEMENT(ArtScaner)
			NAME2TYPE_ELEMENT(ArtSpeed)
			NAME2TYPE_ELEMENT(ArtSplinter)
			NAME2TYPE_ELEMENT(ArtTransmitter)
			NAME2TYPE_ELEMENT(ArtWeaponToSpeed)
			NAME2TYPE_ELEMENT(BK)
			NAME2TYPE_ELEMENT(CB)
			NAME2TYPE_ELEMENT(CargoHook)
			NAME2TYPE_ELEMENT(Cistern)
			NAME2TYPE_ELEMENT(DefGenerator)
			NAME2TYPE_ELEMENT(Engine)
			NAME2TYPE_ELEMENT(Food)
			NAME2TYPE_ELEMENT(FuelTanks)
			NAME2TYPE_ELEMENT(Hull)
			NAME2TYPE_ELEMENT(Kling)
			NAME2TYPE_ELEMENT(Luxury)
			NAME2TYPE_ELEMENT(MC)
			NAME2TYPE_ELEMENT(Medicine)
			NAME2TYPE_ELEMENT(Minerals)
			NAME2TYPE_ELEMENT(Narcotics)
			NAME2TYPE_ELEMENT(Nod)
			NAME2TYPE_ELEMENT(PB)
			NAME2TYPE_ELEMENT(Pirate)
			NAME2TYPE_ELEMENT(Protoplasm)
			NAME2TYPE_ELEMENT(RC)
			NAME2TYPE_ELEMENT(Radar)
			NAME2TYPE_ELEMENT(Ranger)
			NAME2TYPE_ELEMENT(RepairRobot)
			NAME2TYPE_ELEMENT(Satellite)
			NAME2TYPE_ELEMENT(SB)
			NAME2TYPE_ELEMENT(Scaner)
			NAME2TYPE_ELEMENT(Technics)
			NAME2TYPE_ELEMENT(Transport)
			NAME2TYPE_ELEMENT(UselessItem)
			NAME2TYPE_ELEMENT(W01)
			NAME2TYPE_ELEMENT(W02)
			NAME2TYPE_ELEMENT(W03)
			NAME2TYPE_ELEMENT(W04)
			NAME2TYPE_ELEMENT(W05)
			NAME2TYPE_ELEMENT(W06)
			NAME2TYPE_ELEMENT(W07)
			NAME2TYPE_ELEMENT(W08)
			NAME2TYPE_ELEMENT(W09)
			NAME2TYPE_ELEMENT(W10)
			NAME2TYPE_ELEMENT(W11)
			NAME2TYPE_ELEMENT(W12)
			NAME2TYPE_ELEMENT(W13)
			NAME2TYPE_ELEMENT(W14)
			NAME2TYPE_ELEMENT(W15)
			NAME2TYPE_ELEMENT(W16)
			NAME2TYPE_ELEMENT(W17)
			NAME2TYPE_ELEMENT(WB)
			NAME2TYPE_ELEMENT(Warrior)
		END_FOR()

		BEGIN_FOR(GoodsEnum)
			NAME2TYPE_ELEMENT(Food)
			NAME2TYPE_ELEMENT(Medicine)
			NAME2TYPE_ELEMENT(Alcohol)
			NAME2TYPE_ELEMENT(Minerals)
			NAME2TYPE_ELEMENT(Luxury)
			NAME2TYPE_ELEMENT(Technics)
			NAME2TYPE_ELEMENT(Arms)
			NAME2TYPE_ELEMENT(Narcotics)
		END_FOR()
		
		BEGIN_FOR(OwnersGroup)
			NAME2TYPE_ELEMENT(Klings)
			NAME2TYPE_ELEMENT(Normals)
			NAME2TYPE_ELEMENT(Pirates)
		END_FOR()

		BEGIN_FOR(Owner)
			NAME2TYPE_ELEMENT(None)
			NAME2TYPE_ELEMENT(Kling)
			NAME2TYPE_ELEMENT(Fei)
			NAME2TYPE_ELEMENT(Gaal)
			NAME2TYPE_ELEMENT(Maloc)
			NAME2TYPE_ELEMENT(Peleng)
			NAME2TYPE_ELEMENT(People)
			NAME2TYPE_ELEMENT(PirateClan)
		END_FOR()

		#undef BEGIN_FOR
		#undef NAME2TYPE_ELEMENT
		#undef END_FOR
	}

} // namespace enums

namespace kv
{
	void init_storage_impl()
	{
		#define BEGIN_DEF_FOR(struct_name) { using t = struct_name;
		#define ADD_DEF(field) add_definition(&t::field, #field);
		#define ADD_DEF_SPECIFIC(field, key) add_definition(&t::field, #key);
		#define END_DEF() }

		BEGIN_DEF_FOR(Global)
			ADD_DEF(IDay)
		END_DEF()

		BEGIN_DEF_FOR(Player)
			ADD_DEF(ICurStarId)
			ADD_DEF(Debt)
		END_DEF()

		BEGIN_DEF_FOR(Ship)
			ADD_DEF(IFullName)
			ADD_DEF(IType)
			ADD_DEF(Name)
			ADD_DEF(IPlanet)
			ADD_DEF(Goods)
			ADD_DEF(Money)
		END_DEF()

		BEGIN_DEF_FOR(Station)
			ADD_DEF(ShopGoods)
			ADD_DEF(ShopGoodsSale)
			ADD_DEF(ShopGoodsBuy)
		END_DEF()

		BEGIN_DEF_FOR(Item)
			ADD_DEF(IName)
			ADD_DEF(IType)
			ADD_DEF(Size)

			ADD_DEF(Ammo)
			ADD_DEF(Armor)
			ADD_DEF(Bonus)
			ADD_DEF(Broken)
			ADD_DEF(BuiltByPirate)
			ADD_DEF(Capacity)
			ADD_DEF(Cost)
			ADD_DEF(DomSeries)
			ADD_DEF(Durability)
			ADD_DEF(Fuel)
			ADD_DEF(IBonusName)
			ADD_DEF(ISeriesName)
			ADD_DEF(ISpecialName)
			ADD_DEF(Jump)
			ADD_DEF(MaxAmmo)
			ADD_DEF(MaxDamage)
			ADD_DEF(MinDamage)
			ADD_DEF(NoDrop)
			ADD_DEF(Owner)
			ADD_DEF(Power)
			ADD_DEF(Radius)
			ADD_DEF(Repair)
			ADD_DEF(Series)
			ADD_DEF(ShipType)
			ADD_DEF(Special)
			ADD_DEF(Speed)
			ADD_DEF(SpeedMax)
			ADD_DEF(SpeedMin)
			ADD_DEF(SysName)
			ADD_DEF(TechLevel)
			ADD_DEF(X)
			ADD_DEF(Y)
		END_DEF()

		BEGIN_DEF_FOR(HiddenItem)
			ADD_DEF(LandType)
			ADD_DEF(Depth)
		END_DEF()

		BEGIN_DEF_FOR(Planet)
			ADD_DEF(PlanetName)
			ADD_DEF(Owner)
			ADD_DEF(Race)
			ADD_DEF(Economy)
			ADD_DEF(Goverment)
			ADD_DEF(ISize)
			ADD_DEF(OrbitRadius)
			ADD_DEF(OrbitAngle)
			ADD_DEF(RelationToPlayer)
			ADD_DEF(IMainTechLevel)
			ADD_DEF(CurrentInvention)
			ADD_DEF(CurrentInventionPoints)
			ADD_DEF(ShopGoods)
			ADD_DEF(ShopGoodsSale)
			ADD_DEF(ShopGoodsBuy)
		END_DEF()

		BEGIN_DEF_FOR(Star)
			ADD_DEF(StarName)
			ADD_DEF(X)
			ADD_DEF(Y)
			ADD_DEF(Owners)
		END_DEF()

		BEGIN_DEF_FOR(Hole)
			ADD_DEF(Star1Id)
			ADD_DEF(Star2Id)
			ADD_DEF(TurnsToClose)
		END_DEF()

		#undef BEGIN_DEF_FOR
		#undef ADD_DEF
		#undef ADD_DEF_SPECIFIC
		#undef END_DEF
	}
}

struct staticIniter
{
	staticIniter()
	{
		kv::init_storage_impl();
		enums::init_converter_impl();
	}
};

static staticIniter initer;
}