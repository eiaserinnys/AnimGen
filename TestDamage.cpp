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
class DamageCalculatorBase : public IDamageCalculator {
public:
	const EvaluatingSkills& skillsToUse;
	WeaponDesc weapon;
	MonsterDesc monster;

	DamageCalculatorBase(const EvaluatingSkills& skillsToUse)
		: skillsToUse(skillsToUse)
	{
	}

	virtual bool IsCombinationValid(const CombinationBase* comb) const = 0;

	void SetContext(
		const WeaponDesc& weapon,
		const MonsterDesc& monster)
	{
		this->weapon = weapon;
		this->monster = monster;
	}

	double Do(const CombinationBase* comb) const
	{
		if (IsCombinationValid(comb))
		{
			Desc desc(skillsToUse, comb);
			return Calculate(nullptr, weapon, desc, monster);
		}

		return 0;
	}
};

//------------------------------------------------------------------------------
struct ToEvaluate
{
	wstring name;
	wstring skill;
	wstring skillAbb;
	WeaponDesc weapon;
	MonsterDesc monster;
};

//------------------------------------------------------------------------------
void TestDamageA(
	const EvaluatingSkills& evSkills,
	const wstring& evalName, 
	const WeaponDesc& weapon,
	const MonsterDesc& monster,
	DamageCalculatorBase& calc, 
	int maxCount)
{
	DWORD pivotTime = timeGetTime();

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
		{ L"기본", 0.0, 0 },
		{ L"회심", 0.1, 0 },
		{ L"회심,회심", 0.15, 0 },
		{ L"회심,회심,회심", 0.2, 0 },
		{ L"회심,회심,슬롯", 0.15, 1 },
	};

	Custom custom7[] = 
	{
		{ L"기본", 0, 0 }, 
		{ L"회심", 0.10, 0 },
		{ L"회심,회심", 0.15, 0 },
		{ L"회심,슬롯", 0.1, 1 },
	};

	Custom custom8[] =
	{
		{ L"기본", 0, 0 },
		{ L"회심", 0.1, 0 },
		{ L"슬롯", 0.0, 1 },
	};

	{
		map<Armor::PartType, list<GeneralizedArmor*>*> g_generalized;
		FilterArmors(evSkills, g_generalized, true, true);

		list<GeneralizedCombination*> g_all_base;
		PopulateArmors(evSkills, g_generalized, g_all_base, false, true);

		Custom* custom = nullptr;
		int customCount = 0;
		switch (weapon.rarity)
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

			// 무기 슬롯을 확장한다
			for (auto comb : g_all_base)
			{
				auto newComb = new GeneralizedCombination(*comb);

				newComb->slots[0] += weapon.slot1 + custom[j].slot;
				newComb->slots[1] += weapon.slot2;
				newComb->slots[2] += weapon.slot3;

				g_all.push_back(newComb);
			}

			// 대미지 계산기를 만든다
			WeaponDesc modWeapon = weapon;
			modWeapon.criticalRate += custom[j].critical;

			calc.SetContext(modWeapon, monster);

			list<DecoratedCombination*> g_decAll;
			PopulateDecorators(evSkills, g_all, g_decAll, &calc, maxCount, false);

			auto name = Utility::FormatW(
				L"log_damage_%s(%s).txt", 
				evalName.c_str(),
				custom[j].name.c_str());

			_wfopen_s(&file, name.c_str(), L"w,ccs=UNICODE");

			for (auto it = g_decAll.begin(); it != g_decAll.end(); ++it)
			{
				auto comb = *it;

				if (calc.IsCombinationValid(comb))
				{
					Desc desc(evSkills, comb);

					for (int i = 0; i < evSkills.list.size(); ++i)
					{
						fwprintf(
							file,
							L"%d\t",
							comb->skills[i]);
					}

					Calculate(file, modWeapon, desc, monster);

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

//------------------------------------------------------------------------------
void TestDamageElemental()
{
	SkillDescriptor skills_[] =
	{
		{ L"공격", L"공격", 7 },
		{ L"간파", L"간파", 7 },
		{ L"슈퍼 회심", L"슈회", 3, },
		{ L"약점 특효", L"약특", 3, },
		{ L"", L"", 5 },
		{ L"화룡의 비기", L"화비", 2 },

		// 활 강화
		{ L"통상탄/통상화살 강화", L"통강", 1 },
		{ L"산탄/강사 강화", L"산강", 1 },
		{ L"활 모으기 단계 해제", L"활모", 1 },

		// 유틸리티
		{ L"체술", L"체술", 5, },
	};

	ToEvaluate eval[] =
	{
		{
			L"화룡의강궁III",
			L"불속성 공격 강화",
			L"불강",
			WeaponDesc(7, Core::Vector2D(216, 240), 0.2, 1, 0, 0, true, 1.2, false),

			// 키린
			MonsterDesc{ Core::Vector2D(0.45, 0.25), 0.66, Core::Vector2D(0.22, 0.2) },
		},

		{
			L"쟈나프아르카우스III",
			L"불속성 공격 강화",
			L"불강",
			WeaponDesc(6, Core::Vector2D(240, 390), -0.2, 0, 0, 0, true, 1.2, false),

			// 키린
			MonsterDesc{ Core::Vector2D(0.45, 0.25), 0.66, Core::Vector2D(0.22, 0.2) },
		},

		{
			L"워터슛III",
			L"물속성 공격 강화",
			L"물강",
			WeaponDesc(6, Core::Vector2D(204, 240), 0.0, 0, 0, 1, true, 1.2, false),

			// 테오
			MonsterDesc{ Core::Vector2D(0.5, 0.3), 0.66, Core::Vector2D(0.2, 0.15) },
		},

		{
			L"비뢰궁",
			L"번개속성 공격 강화",
			L"뇌강",
			WeaponDesc(7, Core::Vector2D(204, 270), 0.15, 1, 0, 0, true, 1.2, false),
			MonsterDesc{ Core::Vector2D(0.45, 0.2), 0.66, Core::Vector2D(0.25, 0.15), },		// 크샬다오라
		},

		{
			L"레이펀라이크",
			L"얼음속성 공격 강화",
			L"얼강",
			WeaponDesc(8, Core::Vector2D(180, 390), 0.0, 0, 0, 0, true, 1.2, false),

			// 제노지바 임계 머리/비임계 배
			//MonsterDesc{ Core::Vector2D(0.6, 0.3), 0.66, Core::Vector2D(0.25, 0.1) },

			// 테오
			MonsterDesc{ Core::Vector2D(0.5, 0.25), 0.66, Core::Vector2D(0.2, 0.1) },
		},

		{
			L"알나스다오라",
			L"얼음속성 공격 강화",
			L"얼강",
			WeaponDesc(8, Core::Vector2D(204, 240), 0.1, 1, 1, 0, true, 1.2, false),

			// 제노지바 임계 머리/비임계 배
			//MonsterDesc{ Core::Vector2D(0.6, 0.3), 0.66, Core::Vector2D(0.25, 0.1) },

			// 테오
			MonsterDesc{ Core::Vector2D(0.5, 0.25), 0.66, Core::Vector2D(0.2, 0.1) },
		},

		{
			L"필멸의화살",
			L"용속성 공격 강화",
			L"용강",
			WeaponDesc(8, Core::Vector2D(240, 150), 0.0, 1, 0, 0, true, 1.2, false),

			// 리오레우스 머리
			MonsterDesc{ Core::Vector2D(0.60, 0.30), 0.66, Core::Vector2D(0.2, 0.2) },
		},

		{
			L"용골궁III",
			L"용속성 공격 강화",
			L"용강",
			WeaponDesc(6, Core::Vector2D(180, 420), 0.0, 2, 0, 0, false, 1.2, false),

			// 리오레우스 머리
			MonsterDesc{ Core::Vector2D(0.60, 0.30), 0.66, Core::Vector2D(0.2, 0.2) },
		},

		{
			L"하자크베로스II",
			L"용속성 공격 강화",
			L"용강",
			WeaponDesc(7, Core::Vector2D(228, 270), 0.0, 0, 1, 0, true, 1.2, false),

			// 리오레우스 머리
			MonsterDesc{ Core::Vector2D(0.60, 0.30), 0.66, Core::Vector2D(0.2, 0.2) },
		},

		{
			L"제노메토라",
			L"용속성 공격 강화",
			L"용강",
			WeaponDesc(8, Core::Vector2D(204, 180), 0.15, 0, 0, 2, true, 1.2, false),

			// 리오레우스 머리
			MonsterDesc{ Core::Vector2D(0.60, 0.30), 0.66, Core::Vector2D(0.2, 0.2) },
		},
	};

	class DamageCalc : public DamageCalculatorBase {
	public:
		DamageCalc(const EvaluatingSkills& evSkills)
			: DamageCalculatorBase(evSkills)
		{
		}

		virtual bool IsCombinationValid(const CombinationBase* comb) const
		{
			bool arrow =
				(comb->skills[6] == 1 && comb->skills[7] == 1) ||
				(comb->skills[6] == 0 && comb->skills[7] == 0) ||
				skillsToUse.list[6].name != L"통상탄/통상화살 강화" ||
				skillsToUse.list[7].name != L"산탄/강사 강화";

			bool gambit =
				(comb->skills[5] >= 2 || comb->skills[5] == 0) ||
				skillsToUse.list[5].name != L"화룡의 비기";

			bool move =
				comb->skills[9] >= 3 ||
				skillsToUse.list[9].name != L"체술";

			return arrow && gambit && move;
		}
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

		DamageCalc calc(evSkills);

		TestDamageA(
			evSkills, 
			eval[i].name, 
			eval[i].weapon, 
			eval[i].monster,
			calc,
			15);
	}
}

//------------------------------------------------------------------------------
void TestDragonPiercer()
{
	SkillDescriptor skills_[] =
	{
		{ L"공격", L"공격", 7 },
		{ L"간파", L"간파", 7 },
		{ L"슈퍼 회심", L"슈회", 3, },
		{ L"약점 특효", L"약특", 3, },
		{ L"무속성 강화", L"무강", 1 },

		{ L"발도술 [기]", L"발도", 3 }, 
		{ L"납도술", L"납도", 3 },

		// 활 강화
		{ L"특수 사격 강화", L"특강", 2 },
		{ L"관통탄/용화살 강화", L"관강", 1 }, 
		{ L"활 모으기 단계 해제", L"활모", 1 },
	};

	auto weapon = WeaponDesc(
		8, Core::Vector2D(264, 0), -0.3, 2, 0, 0, true, 1.2, true);

	auto monster =
		// 제노지바 임계 가슴, 통상 꼬리
		MonsterDesc
		{
			Core::Vector2D(0.7, 0.0), 0.4,
			Core::Vector2D(0.3, 0.0) 
		};

	class DamageCalc : public DamageCalculatorBase {
	public:
		DamageCalc(const EvaluatingSkills& evSkills)
			: DamageCalculatorBase(evSkills)
		{
		}

		virtual bool IsCombinationValid(const CombinationBase* comb) const
		{
			bool undraw =
				comb->skills[6] >= 3 ||
				skillsToUse.list[6].name != L"납도술";

			return undraw;
		}
	};

	EvaluatingSkills evSkills;
	{
		evSkills.list.insert(evSkills.list.end(), skills_, skills_ + COUNT_OF(skills_));
		FilterDecorators(evSkills);
		FilterCharms(evSkills);
		evSkills.Update();
	}

	DamageCalc calc(evSkills);

	TestDamageA(evSkills, L"각왕궁게일혼", weapon, monster, calc, 15);
}

//------------------------------------------------------------------------------
void TestMutedDragonPiercer()
{
	SkillDescriptor skills_[] =
	{
		{ L"공격", L"공격", 7 },
		{ L"간파", L"간파", 7 },
		{ L"슈퍼 회심", L"슈회", 3, },
		{ L"약점 특효", L"약특", 3, },
		{ L"무속성 강화", L"무강", 1 },

		{ L"발도술 [기]", L"발도", 3 },
		{ L"납도술", L"납도", 3 },

		// 활 강화
		{ L"특수 사격 강화", L"특강", 2 },
		{ L"관통탄/용화살 강화", L"관강", 1 },
		{ L"활 모으기 단계 해제", L"활모", 1 },

		{ L"귀마개", L"방음", 5 },
	};

	auto weapon = WeaponDesc(
		8, Core::Vector2D(264, 0), -0.3, 2, 0, 0, true, 1.2, true);

	auto monster =
		// 제노지바 임계 가슴, 통상 꼬리
		MonsterDesc
	{
		Core::Vector2D(0.7, 0.0), 0.4,
		Core::Vector2D(0.3, 0.0)
	};

	class DamageCalc : public DamageCalculatorBase {
	public:
		DamageCalc(const EvaluatingSkills& evSkills)
			: DamageCalculatorBase(evSkills)
		{
		}

		virtual bool IsCombinationValid(const CombinationBase* comb) const
		{
			bool undraw =
				comb->skills[6] >= 3 ||
				skillsToUse.list[6].name != L"납도술";

			bool mute =
				comb->skills[10] >= 5 ||
				skillsToUse.list[10].name != L"귀마개";

			return undraw && mute;
		}
	};

	EvaluatingSkills evSkills;
	{
		evSkills.list.insert(evSkills.list.end(), skills_, skills_ + COUNT_OF(skills_));
		FilterDecorators(evSkills);
		FilterCharms(evSkills);
		evSkills.Update();
	}

	DamageCalc calc(evSkills);

	TestDamageA(evSkills, L"[방음]각왕궁게일혼", weapon, monster, calc, 15);
}

//------------------------------------------------------------------------------
void TestDamage()
{
	LoadArmors(false);
	LoadDecorators();
	LoadCharms();

	TestDragonPiercer();
	TestMutedDragonPiercer();
	TestDamageElemental();
}