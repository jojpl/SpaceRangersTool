#include "Entities.h"
#include "convert.h"

#include <algorithm>
#include <map>
#include <string_view>

#include <cctype>

namespace Entities
{

	template<>
	Type str2type(std::string_view sw)
	{
		#define NAME2TYPE_ELEMENT(name) {#name, Type::name}
		static std::map< std::string_view, Type> map
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

		return map[sw];
	}

	GoodsQty unpack_goods_str(std::string_view sw)
	{
		GoodsQty goods;
		const char* beg = sw.data();
		const char* end = sw.data() + sw.size();
		const char* p1 = beg;
		const char* p2 = beg;

		int cnt = 0;
		while (p1 !=end)
		{
			while ((p1 != end) && !isdigit(*p1)) p1++;
			p2 = p1;
			while ((p2 != end) && isdigit(*p2)) p2++;

			int res = conv::to_int(std::string_view(p1, p2 - p1));
			p1 = p2;

			goods.packed[GoodsEnum(cnt)] = res;
			cnt++;
		}
		return goods;
	}
}