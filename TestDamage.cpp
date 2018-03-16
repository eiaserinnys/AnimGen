#include "pch.h"

#include <timeapi.h>

#include <algorithm>

#include <locale.h>
#include <Utility.h>
#include <WindowsUtility.h>

#include "Vector.h"

#include "Skill.h"
#include "Decorator.h"
#include "Armor.h"
#include "Charm.h"
#include "GeneralizedArmor.h"
#include "DamageCalculation.h"
#include "DecoratedCombination.h"

using namespace std;
//using namespace Core;

static FILE* file = nullptr;

//------------------------------------------------------------------------------
class DamageCalculator : public IDamageCalculator {
public:
	WeaponDesc weapon;
	MonsterDesc monster;

	DamageCalculator(
		const WeaponDesc& weapon,
		const MonsterDesc& monster)
		: weapon(weapon), monster(monster)
	{
	}

	static bool IsCombinationValid(const CombinationBase* comb)
	{
		bool arrow =
			(comb->skills[6] == 1 && comb->skills[7] == 1) ||
			(comb->skills[6] == 0 && comb->skills[7] == 0) ||
			g_skills[6] != L"통상탄/통상화살 강화" ||
			g_skills[7] != L"산탄/강사 강화";

		bool gambit =
			(comb->skills[5] >= 2 || comb->skills[5] == 0) ||
			g_skills[5] != L"화룡의 비기";

		bool move =
			comb->skills[9] >= 3 ||
			g_skills[9] != L"체술";

		return arrow && gambit && move;
	}

	double Do(const CombinationBase* comb) const
	{
		if (IsCombinationValid(comb))
		{
			Desc desc(comb);
			return Calculate(nullptr, weapon, desc, monster);
		}

		return 0;
	}
};

//------------------------------------------------------------------------------
void TestDamage()
{
	DWORD pivotTime = timeGetTime();

	LoadArmors(false);
	LoadDecorators();
	LoadCharms();
	CheckActiveSkills();

	map<Armor::PartType, list<GeneralizedArmor*>*> g_generalized;
	FilterArmors(g_generalized, true, true);

	list<GeneralizedCombination*> g_all_base;
	PopulateArmors(g_generalized, g_all_base, false, true);

	struct ToEvaluate
	{
		string name;
		WeaponDesc weapon;
		MonsterDesc monster;
	};

	ToEvaluate eval[] = 
	{
		{
			"비뢰궁(기본)", 
			WeaponDesc(Core::Vector2D(180, 390), 0.0, 0, 0, 0, 1.2),  
			MonsterDesc { 0.6, 0.4 }, 
		},
		{
			"비뢰궁(회심,회심)",
			WeaponDesc(Core::Vector2D(180, 390), 0.15, 0, 0, 0, 1.2),
			MonsterDesc{ 0.6, 0.4 },
		},
		{
			"비뢰궁(회심,슬롯)",
			WeaponDesc(Core::Vector2D(180, 390), 0.1, 1, 0, 0, 1.2),
			MonsterDesc{ 0.6, 0.4 },
		},
	};

	for (int i = 0; i < COUNT_OF(eval); ++i)
	{
		list <GeneralizedCombination*> g_all;

		// 무기 슬롯을 확장한다
		for (auto comb : g_all_base)
		{
			auto newComb = new GeneralizedCombination(*comb);

			newComb->slots[0] += eval[i].weapon.slot1;
			newComb->slots[1] += eval[i].weapon.slot2;
			newComb->slots[2] += eval[i].weapon.slot3;

			g_all.push_back(newComb);
		}

		// 대미지 계산기를 만든다
		DamageCalculator calc(eval[i].weapon, eval[i].monster);

		list<DecoratedCombination*> g_decAll;
		PopulateDecorators(g_all, g_decAll, &calc, false);

		auto name = Utility::Format("log_damage_%s.txt", eval[i].name.c_str());

		fopen_s(&file, name.c_str(), "w,ccs=UNICODE");

		for (auto it = g_decAll.begin(); it != g_decAll.end(); ++it)
		{
			auto comb = *it;

			if (DamageCalculator::IsCombinationValid(comb))
			{
				Desc desc(comb);

				for (int i = 0; i < COUNT_OF(g_skills); ++i)
				{
					fwprintf(
						file,
						L"%d\t",
						comb->skills[i]);
				}

				Calculate(file, eval[i].weapon, desc, eval[i].monster);

				comb->Write(file);

				fwprintf(file, L"\n");
			}
		}

		fclose(file);

		for (auto comb : g_all) { comb->Delete(); }

		WindowsUtility::Debug(L"%dsec elapsed", (timeGetTime() - pivotTime) / 1000);
	}
}