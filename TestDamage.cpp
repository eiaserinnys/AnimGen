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
	const EvaluatingSkills& skillsToUse;
	WeaponDesc weapon;
	MonsterDesc monster;

	DamageCalculator(
		const EvaluatingSkills& skillsToUse,
		const WeaponDesc& weapon,
		const MonsterDesc& monster)
		: skillsToUse(skillsToUse), weapon(weapon), monster(monster)
	{
	}

	bool IsCombinationValid(const CombinationBase* comb) const
	{
		bool arrow =
			(comb->skills[6] == 1 && comb->skills[7] == 1) ||
			(comb->skills[6] == 0 && comb->skills[7] == 0) ||
			skillsToUse.list[6].name != L"���ź/���ȭ�� ��ȭ" ||
			skillsToUse.list[7].name != L"��ź/���� ��ȭ";

		bool gambit =
			(comb->skills[5] >= 2 || comb->skills[5] == 0) ||
			skillsToUse.list[5].name != L"ȭ���� ���";

		bool move =
			comb->skills[9] >= 3 ||
			skillsToUse.list[9].name != L"ü��";

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

	SkillDescriptor skills_[] =
	{
		{ L"����", L"����", 7 },
		{ L"����", L"����", 7 },
		{ L"���� ȸ��", L"��ȸ", 3, },
		{ L"���� Ưȿ", L"��Ư", 3, },
		{ L"", L"", 5 },
		{ L"ȭ���� ���", L"ȭ��", 2 },

		// Ȱ ��ȭ
		{ L"���ź/���ȭ�� ��ȭ", L"�밭", 1 },
		{ L"��ź/���� ��ȭ", L"�갭", 1 },
		{ L"Ȱ ������ �ܰ� ����", L"Ȱ��", 1 },

		// ��ƿ��Ƽ
		{ L"ü��", L"ü��", 5, },
	};

	struct ToEvaluate
	{
		wstring name;
		wstring skill;
		wstring skillAbb;
		WeaponDesc weapon;
		MonsterDesc monster;
	};

	ToEvaluate eval[] = 
	{
		{
			L"�����Ƹ�ī�콺III",
			L"�ҼӼ� ���� ��ȭ",
			L"�Ұ�",
			WeaponDesc(6, Core::Vector2D(240, 390), -0.2, 0, 0, 0, 1.2),
			MonsterDesc{ 0.45, 0.25 },		// Ű��
		},

		{
			L"���ͽ�III",
			L"���Ӽ� ���� ��ȭ",
			L"����",
			WeaponDesc(6, Core::Vector2D(204, 240), 0.0, 0, 0, 1, 1.2),
			MonsterDesc{ 0.5, 0.3 },		// �׿�
		},

		{
			L"��ڱ�", 
			L"�����Ӽ� ���� ��ȭ",
			L"����",
			WeaponDesc(7, Core::Vector2D(204, 270), 0.15, 1, 0, 0, 1.2),  
			MonsterDesc{ 0.45, 0.2, },		// ũ���ٿ���
		},

		{
			L"�����ݶ���ũ",
			L"�����Ӽ� ���� ��ȭ",
			L"������",
			WeaponDesc(8, Core::Vector2D(180, 390), 0.0, 0, 0, 0, 1.2),
			MonsterDesc{ 0.6, 0.3 },		// �������� �Ӱ�
		},

		{
			L"����ũ���ν�II",
			L"��Ӽ� ���� ��ȭ",
			L"�밭",
			WeaponDesc(7, Core::Vector2D(228, 270), 0.0, 0, 1, 0, 1.2),
			MonsterDesc{ 0.6, 0.25 },		// ������ũ �Ӹ� �ı�
		},
	};

	struct Custom
	{
		Custom() = default;
		Custom(const wstring& name, double critical, int slot)
			: name(name), critical(critical), slot(slot)
		{}

		wstring name;
		double critical = 0;
		int slot = 0;
	};

	Custom custom6[] =
	{
		{ L"�⺻", 0.0, 0 },
		{ L"ȸ��,ȸ��,ȸ��", 0.2, 0 },
		{ L"ȸ��,ȸ��,����", 0.15, 1 },
	};

	Custom custom7[] = 
	{
		{ L"�⺻", 0, 0 }, 
		{ L"ȸ��,ȸ��", 0.15, 0 },
		{ L"ȸ��,����", 0.1, 1 },
	};

	Custom custom8[] =
	{
		{ L"�⺻", 0, 0 },
		{ L"ȸ��", 0.1, 0 },
		{ L"����", 0.0, 1 },
	};

	for (int i = 0; i < COUNT_OF(eval); ++i)
	{
		WindowsUtility::Debug(L"Evaluating %s...\n", eval[i].name.c_str());

		EvaluatingSkills evSkills;
		{
			evSkills.list.insert(evSkills.list.end(), skills_, skills_ + COUNT_OF(skills_));
			evSkills.list[4].name = eval[i].skill;
			evSkills.list[4].abbName = eval[i].skillAbb;

			FilterDecorators(evSkills);
			FilterCharms(evSkills);
			evSkills.Update();
		}

		map<Armor::PartType, list<GeneralizedArmor*>*> g_generalized;
		FilterArmors(evSkills, g_generalized, true, true);

		list<GeneralizedCombination*> g_all_base;
		PopulateArmors(evSkills, g_generalized, g_all_base, false, true);

		Custom* custom = nullptr;
		int customCount = 0;
		switch (eval[i].weapon.rarity)
		{
		case 6:
			custom = custom6;
			customCount = COUNT_OF(custom6);
			break;

		case 7:
			custom = custom7;
			customCount = COUNT_OF(custom7);
			break;

		case 8:
			custom = custom8;
			customCount = COUNT_OF(custom8);
			break;
		}

		for (int j = 0; j < customCount; ++j)
		{
			WindowsUtility::Debug(L"\twith custom %s\n", custom[j].name.c_str());

			list <GeneralizedCombination*> g_all;

			// ���� ������ Ȯ���Ѵ�
			for (auto comb : g_all_base)
			{
				auto newComb = new GeneralizedCombination(*comb);

				newComb->slots[0] += eval[i].weapon.slot1 + custom[j].slot;
				newComb->slots[1] += eval[i].weapon.slot2;
				newComb->slots[2] += eval[i].weapon.slot3;

				g_all.push_back(newComb);
			}

			// ����� ���⸦ �����
			WeaponDesc modWeapon = eval[i].weapon;
			modWeapon.criticalRate += custom[j].critical;

			DamageCalculator calc(evSkills, modWeapon, eval[i].monster);

			list<DecoratedCombination*> g_decAll;
			PopulateDecorators(evSkills, g_all, g_decAll, &calc, false);

			auto name = Utility::FormatW(
				L"log_damage_%s(%s).txt", 
				eval[i].name.c_str(),
				custom[j].name.c_str());

			_wfopen_s(&file, name.c_str(), L"w,ccs=UNICODE");

			for (auto it = g_decAll.begin(); it != g_decAll.end(); ++it)
			{
				auto comb = *it;

				if (calc.IsCombinationValid(comb))
				{
					Desc desc(comb);

					for (int i = 0; i < evSkills.list.size(); ++i)
					{
						fwprintf(
							file,
							L"%d\t",
							comb->skills[i]);
					}

					Calculate(file, modWeapon, desc, eval[i].monster);

					comb->Write(file);

					fwprintf(file, L"\n");
				}
			}

			fclose(file);

			for (auto comb : g_all) { comb->Delete(); }

			WindowsUtility::Debug(L"%dsec elapsed", (timeGetTime() - pivotTime) / 1000);
		}

		for (auto cont : g_generalized)
		{
			for (auto g : *cont.second) { g->Delete(); }
			delete cont.second;
		}

		for (auto g : g_all_base) { g->Delete(); }
		g_all_base.clear();
	}
}