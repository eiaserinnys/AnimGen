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

	if (false)
	{
		CombinationBase a(COUNT_OF(g_skills)), b(COUNT_OF(g_skills));
		a.skills[0] = 1;
		a.skills[1] = 1;
		b.slots[0] = 1;

		auto comp = CombinationBase::CompareStrict2(a, b);

		comp = CombinationBase::CompareStrict2(a, b);
	}
	
	map<Armor::PartType, list<GeneralizedArmor*>*> g_generalized;
	FilterArmors(g_generalized, true, true);

	list<GeneralizedCombination*> g_all;
	PopulateArmors(g_generalized, g_all, false, true);

	MonsterDesc monster;
	WeaponDesc weapon;

	// 무기 슬롯까지 확장한다
	for (auto comb : g_all)
	{
		comb->slots[0] += weapon.slots[0];
		comb->slots[1] += weapon.slots[1];
		comb->slots[2] += weapon.slots[2];
	}

	// 대미지 계산기를 만든다
	DamageCalculator calc(weapon, monster);

	list<DecoratedCombination*> g_decAll;
	PopulateDecorators(g_all, g_decAll, &calc, false);

	fopen_s(&file, "log_damage.txt", "w,ccs=UNICODE");

	for (auto it = g_decAll.begin(); it != g_decAll.end(); ++it)
	{
		auto comb = *it;

		//0L"공격",
		//1L"간파",
		//2L"슈퍼 회심",
		//3L"약점 특효",
		//4L"번개속성 공격 강화",
		//5L"화룡의 비기",
		//6L"통상탄/통상화살 강화",
		//7L"산탄/강사 강화",
		//8L"활 모으기 단계 해제",
		//9L"체술",

		if (DamageCalculator::IsCombinationValid(comb))
		{
			Desc desc(comb);

			fwprintf(
				file,
				L"%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",
				comb->skills[0],
				comb->skills[1],
				comb->skills[2],
				comb->skills[3],
				comb->skills[4],
				comb->skills[5],
				comb->skills[6],
				comb->skills[7],
				comb->skills[8],
				comb->skills[9]);

			Calculate(file, weapon, desc, monster);

			comb->Write(file);

			fwprintf(file, L"\n");
		}
	}

#if 0
	for (int a = 0; a <= MaxAttackSkillLevel(); ++a)
	{
		desc.attackBonus = AttackSkillBonus(a);

		for (int b = 0; b <= MaxElementalSkillLevel(); ++b)
		{
			desc.elementalBonus = ElementalSkillLevel(b);

			for (int c = 0; c <= MaxCriticalEyeSkillLevel(); ++c)
			{
				desc.criticalEye = CriticalEye(c);

				for (int d = 0; d <= MaxSuperCriticalSkillLevel(); ++d)
				{
					desc.superCritical = SuperCritical(d);

					for (int e = 0; e <= MaxExploitWeaknessSkillLevel(); ++e)
					{
						desc.exploitWeakness = ExploitWeakness(e);

						for (int f = 0; f < 2; ++f)
						{
							desc.fireDragonGambit = f == 0 ? 0 : FireDragonsGambit();

							fprintf(
								file,
								"%d\t%d\t%d\t%d\t%d\t%d\t",
								a, b, c, d, e, f);

							Calculate(desc);
						}
					}
				}
			}

		}
	}
#endif

	fclose(file);

	WindowsUtility::Debug(L"%dsec elapsed", (timeGetTime() - pivotTime) / 1000);
}