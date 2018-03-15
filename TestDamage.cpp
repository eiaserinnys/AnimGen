#include "pch.h"

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

using namespace std;
//using namespace Core;

static FILE* file = nullptr;

list<GeneralizedCombination*> g_all;

//------------------------------------------------------------------------------
template <typename CombinationType>
int RejectWorseCombinations(
	list<CombinationType*>& next, 
	bool dump)
{
	// 생성된 페어를 평가한다
	int rejected = 0;
	for (auto it = next.begin(); it != next.end(); )
	{
		auto toEvaluate = *it;

		bool bad = false;

		auto jt = it;
		jt++;

		for (; jt != next.end();)
		{
			auto target = *jt;

			auto result3 = GeneralizedCombination::CompareStrict2(*toEvaluate, *target);

#if 0
			{
				auto result = toEvaluate->Compare(*target);
				auto result2 = target->Compare(*toEvaluate);

				if (result == GeneralizedCombination::Equal)
				{
					assert(result2 == GeneralizedCombination::Equal);
					assert(result3 == GeneralizedCombination::Equal);
				}
				else if (result == GeneralizedCombination::Worse)
				{
					assert(result2 == GeneralizedCombination::NotWorse);
					assert(result3 == GeneralizedCombination::Worse);
				}
				else if (result == GeneralizedCombination::NotWorse)
				{
					if (result2 == GeneralizedCombination::NotWorse)
					{
						result3 = GeneralizedCombination::Compare(*toEvaluate, *target);
						assert(result3 == GeneralizedCombination::Undetermined);
					}
					else if (result2 == GeneralizedCombination::Worse)
					{
						assert(result3 == GeneralizedCombination::Better);
					}
					else
					{
						auto result2 = target->Compare(*toEvaluate);
						assert(!"impossible");
					}
				}
			}
#endif

			if (result3 == GeneralizedCombination::Equal)
			{
				// 우측을 좌측에 더하자
				toEvaluate->CombineEquivalent(*target);

				if (dump)
				{
					WindowsUtility::Debug(L"Dropping '");
					target->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"'. (Same)\n");
				}

				++rejected;
				jt = next.erase(jt);
			}
			else if (result3 == GeneralizedCombination::Worse)
			{
				if (dump)
				{
					WindowsUtility::Debug(L"Dropping '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					target->DumpSimple();
					WindowsUtility::Debug(L"'. (Worse)\n");
				}

				bad = true;
				break;
			}
			else if (result3 == GeneralizedCombination::Better)
			{
				if (dump)
				{
					WindowsUtility::Debug(L"Dropping '");
					target->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"'. (Worse)\n");
				}

				++rejected;
				target->Delete();
				jt = next.erase(jt);
			}
			else // Undetermined
			{
				jt++;
			}
		}

		if (bad)
		{
			++rejected;
			toEvaluate->Delete();
			it = next.erase(it);
		}
		else
		{
			it++;
		}
	}

	return rejected;
}

//------------------------------------------------------------------------------
list<DecoratedCombination*> g_decAll;

void PopulateDecorators(bool dumpComparison)
{
	FILE* file;

	fopen_s(&file, "log_decorated.txt", "w,ccs=UNICODE");

	// 데커레이티드 컴비네이션을 새로 만든다
	for (auto comb : g_all)
	{
		g_decAll.push_back(DecoratedCombination::DeriveFrom(comb));
	}

	// 작은 소켓부터 채워가며 리스트를 구축해보자
	// 소켓 크기에 대해서 순회하는 것에 주의
	bool first = true;

	for (int socket = 2; socket >= 0; )
	{
		if (first)
		{
			fwprintf(file, L"Filling socket with size %d\n", socket + 1);
			WindowsUtility::Debug(L"Filling socket with size %d\n", socket + 1);
			first = false;
		}

		list<DecoratedCombination*> next;

		// 먼저 더 슬롯에 끼울 게 없는 조합을 옮긴다
		for (auto it = g_decAll.begin(); it != g_decAll.end(); )
		{
			auto cur = *it;
			if (cur->slots[socket] <= 0)
			{
				next.push_back(cur);
				it = g_decAll.erase(it);
			}
			else
			{
				it++;
			}
		}

		fwprintf(file, L"%d combinations moved\n", (int) next.size());
		WindowsUtility::Debug(L"%d combinations moved\n", next.size());

		int pass = next.size();
		int evaluated = 0;
		int rejected[] = { 0, 0, 0, };
		for (auto it = g_decAll.begin(); it != g_decAll.end(); ++it)
		{
			auto cur = *it;
			assert(cur->slots[socket] > 0);

			auto TryToAdd = [&](Decorator* dec, int decIndex, int socket)
			{
				evaluated++;

				if (evaluated % 10000 == 0)
				{
					WindowsUtility::Debug(
						L"\t%d (passed=%d,evaluated=%d)\n",
						next.size(),
						pass,
						evaluated);

					fflush(file);
				}

				int skillToAdd = dec != nullptr ? dec->skillIndex : - 1;

				auto newCombination = DecoratedCombination::DeriveFrom(cur, dec, socket, decIndex);;

				AddIfNotWorse(next, newCombination, false, file, dumpComparison);
			};

			// 장식주를 넣지 않는 베리에이션도 시도한다
			TryToAdd(nullptr, -1, socket);

			int dFrom = cur->lastSocket == socket ? cur->lastDecoratorIndex : 0;

			for (int d = dFrom; d < g_decorators.size(); ++d)
			{
				auto dec = g_decorators[d];

				if (dec->slotSize != socket + 1) { continue; }

				auto si = dec->skillIndex;
				if (cur->skills[si] + 1 <= g_skillMaxLevel[si])
				{
					TryToAdd(dec, d, socket);
				}
			}
		}

		// 중복된 페어를 제거한다
		//rejected1 = RejectWorseCombinations(next, false);
		//assert(rejected1 == 0);

		for (auto inst : g_decAll) { inst->Delete(); }

		g_decAll.swap(next);

		bool needToIterate = false;
		for (auto inst : g_decAll)
		{
			needToIterate = inst->slots[socket] > 0;
			if (needToIterate) { break; }
		}

		if (!needToIterate) 
		{ 
			socket--;
			first = true;
		}

		fwprintf(
			file, 
			L"\t%d (passed=%d,evaluated=%d)\n",
			(int) g_decAll.size(),
			pass,
			evaluated);

		WindowsUtility::Debug(
			L"\t%d (passed=%d,evaluated=%d)\n",
			g_decAll.size(),
			pass, 
			evaluated);
	}

	fclose(file);
}

//------------------------------------------------------------------------------
void TestDamage()
{
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

	PopulateArmors(g_generalized, g_all, false, true);

	WeaponDesc weapon;
	MonsterDesc monster;

	// 무기 슬롯을 더 뚫는다
	for (auto it = g_all.begin(); it != g_all.end(); ++it)
	{
		auto comb = *it;
		comb->slots[0] += weapon.slots[0];
		comb->slots[1] += weapon.slots[1];
		comb->slots[2] += weapon.slots[2];
	}

	PopulateDecorators(true);

	fopen_s(&file, "log_damage.txt", "w,ccs=UNICODE");

	Desc desc;

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

		if (arrow && gambit && move)
		{
			desc.attackBonus = AttackSkillBonus(comb->skills[0]);
			desc.criticalEye = CriticalEye(comb->skills[1]);
			desc.superCritical = SuperCritical(comb->skills[2]);
			desc.exploitWeakness = ExploitWeakness(comb->skills[3]);
			desc.elementalBonus = ElementalSkillLevel(comb->skills[4]);
			desc.fireDragonGambit = comb->skills[5] >= 2 ? FireDragonsGambit() : 0;
			desc.arrowUpgrade = comb->skills[6] >= 1 && comb->skills[7] >= 1 ? 0.1 : 0;
			desc.chargeLevel = comb->skills[8] >= 1 ? 3 : 2;

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
}