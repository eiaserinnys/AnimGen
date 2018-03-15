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
	// ������ �� ���Ѵ�
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
				// ������ ������ ������
				toEvaluate->CombineEquivalent(target);

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
void PopulateArmors(
	const map<Armor::PartType, std::vector<GeneralizedArmor*>*>& g_generalized)
{
	bool dump = false;

	// �ʱ� ����Ʈ�� �����
	{
		for (int i = 0; i < g_charms.size(); ++i)
		{
			auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
			newComb->Combine(nullptr, g_charms[i]);
			g_all.push_back(newComb);
		}

		WindowsUtility::Debug(L"%d\n", g_all.size());

		// ��ü ���� ����
		for (auto it = g_all.begin(); it != g_all.end(); ++it)
		{
			(*it)->Dump();
		}
	}

	// ���� ä���� �߰��ؼ� �ø���
	for (int channel = 0; channel < Armor::Count; ++channel)
	{
		list<GeneralizedCombination*> next;

		auto it = g_generalized.find((Armor::PartType) channel);
		if (it == g_generalized.end())
		{
			throw invalid_argument("");
		}

		auto& gs = *it->second;

		// ��� ���� x ������ �� �ϴ� ����
		for (auto it = g_all.begin(); it != g_all.end(); ++it)
		{
			for (int i = 0; i < gs.size(); ++i)
			{
				auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
				newComb->Combine(*it, channel, gs[i]);
				next.push_back(newComb);
			}
		}

		// ��ü ���� ����
		int rejected = RejectWorseCombinations(next, true);

		for (auto it = g_all.begin(); it != g_all.end(); ++it)
		{
			delete *it;
		}

		g_all.swap(next);

		WindowsUtility::Debug(L"%d (%d)---------------------------------------\n", g_all.size(), rejected);
	}
	
	WindowsUtility::Debug(L"%d\n", g_all.size());
}

//------------------------------------------------------------------------------
list<DecoratedCombination*> g_decAll;

void PopulateDecorators(bool strict)
{
	// ��Ŀ����Ƽ�� �ĺ���̼��� ���� �����
	for (auto comb : g_all)
	{
		g_decAll.push_back(DecoratedCombination::DeriveFrom(comb));
	}

	auto rejected = RejectWorseCombinations(g_decAll, false);
	WindowsUtility::Debug(
		L"\tInitial Rejection: %d (%d)\n",
		g_decAll.size(),
		rejected);

	// ���� ���Ϻ��� ä������ ����Ʈ�� �����غ���
	// ���� ũ�⿡ ���ؼ� ��ȸ�ϴ� �Ϳ� ����
	bool first = true;

	for (int socket = 0; socket < 3; )
	{
		if (first)
		{
			WindowsUtility::Debug(L"Filling socket with size %d\n", socket + 1);
			first = false;
		}

		list<DecoratedCombination*> next;

		// ���� �� ���Կ� ���� �� ���� ������ �ű��
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

		WindowsUtility::Debug(L"%d combinations moved\n", next.size());

		int pass = next.size();
		int evaluated = 0;
		int rejected[] = { 0, 0, 0, };
		for (auto it = g_decAll.begin(); it != g_decAll.end(); ++it)
		{
			auto cur = *it;
			assert(cur->slots[socket] > 0);

#if 0
			// ��� ���� ������ �� ���� ������ �� ������
			// �̰� �� �� ���� �� ���� ����� ����
			vector<int> decorators;

			auto temp = DecoratedCombination::DeriveFrom(cur);

			function<void(int)> BuildCombination;
			BuildCombination = [&](int iterateFrom)
			{
				for (int d = iterateFrom; d < g_decorators.size(); ++d)
				{
					auto dec = g_decorators[d];

					if (strict)
					{
						if (dec->slotSize != socket + 1) { continue; }
					}
					else
					{
						if (dec->slotSize > socket + 1) { continue; }
					}

					bool notMaxed = 
						temp->skills[dec->skillIndex] + 1 <= 
						g_skillMaxLevel[dec->skillIndex];

					if (!notMaxed) { continue; }

					temp->skills[dec->skillIndex]++;
					temp->slots[socket]--;
					decorators.push_back(d);

					if (temp->slots[socket] > 0)
					{
						BuildCombination(d);
					}
					else
					{
						// ���� �� ������ ����� �־��
						++evaluated;
						if (evaluated % 10000 == 0)
						{
							WindowsUtility::Debug(
								L"\t%d (evaluated=%d,olderdiscarded=%d,equivalent=%d,newerworse=%d)\n",
								next.size(),
								evaluated,
								rejected[0],
								rejected[1],
								rejected[2]);
						}

						auto newComb = DecoratedCombination::DeriveFrom(cur);

						for (int di : decorators)
						{
							auto si = g_decorators[di]->skillIndex;
							newComb->skills[si]++;
							newComb->slots[socket]--;

							assert(newComb->skills[si] <= g_skillMaxLevel[si]);
						}

						assert(newComb->slots[socket] >= 0);

						bool processed = false;
						for (auto it = next.begin(); it != next.end(); )
						{
							auto toCompare = *it;

							// ������ ���� ���¿��� ���Ѵ�
							auto ret = strict ? 
								DecoratedCombination::CompareStrict(*newComb, *toCompare) :
								DecoratedCombination::Compare(*newComb, *toCompare);

							if (ret == DecoratedCombination::Better)
							{
								// ���� �� ����Ʈ�Ѵ�
								toCompare->Delete();
								it = next.erase(it);
								++rejected[0];
							}
							else if (ret == DecoratedCombination::Equal)
							{
								// ���� �� ����Ʈ�Ѵ�
								newComb->CombineEquivalent(toCompare);

								it = next.erase(it);

								++rejected[1];
							}
							else if (ret == DecoratedCombination::Worse)
							{
								++rejected[2];
								processed = true;
								break;
							}
							else
							{
								++it;
							}
						}

						if (!processed)
						{
							next.push_back(newComb);
						}
						else
						{
							newComb->Delete();
						}
					}

					temp->skills[dec->skillIndex]--;
					temp->slots[socket]++;
					decorators.pop_back();
				}
			};

			BuildCombination(0);

			temp->Delete();
#else

			auto TryToAdd = [&](Decorator* dec, int decIndex, int socket)
			{
				evaluated++;

				int skillToAdd = dec != nullptr ? dec->skillIndex : - 1;

				DecoratedCombination* newCombination = nullptr;

				bool processed = false;
				for (auto it = next.begin(); it != next.end(); )
				{
					auto toCompare = *it;

					// ������ ���� ���¿��� ���Ѵ�
					auto ret = strict ?
						DecoratedCombination::CompareStrict(*cur, *toCompare, skillToAdd, socket) :
						DecoratedCombination::Compare(*cur, *toCompare, skillToAdd, socket);

					if (ret == DecoratedCombination::Better)
					{
#if 0
						auto temp = DecoratedCombination::DeriveFrom(cur, dec, socket, decIndex);

						WindowsUtility::Debug(L"Saving\n\t");
						temp->Dump();
						temp->Delete();
						WindowsUtility::Debug(L"for\n\t");
						toCompare->Dump();

						ret = strict ?
							DecoratedCombination::CompareStrict(*cur, *toCompare, skillToAdd, socket) :
							DecoratedCombination::Compare(*cur, *toCompare, skillToAdd, socket);
#endif

						// ���� �� ����Ʈ�Ѵ�
						toCompare->Delete();
						it = next.erase(it);
						++rejected[0];
					}
					else if (ret == DecoratedCombination::Equal)
					{
						if (newCombination == nullptr)
						{
							newCombination = DecoratedCombination::DeriveFrom(cur, dec, socket, decIndex);
						}

						// ���� �� ����Ʈ�Ѵ�
						newCombination->CombineEquivalent(toCompare);

						it = next.erase(it);

						++rejected[1];
					}
					else if (ret == DecoratedCombination::Worse)
					{
						++rejected[2];
						processed = true;
						break;
					}
					else
					{
						++it;
					}
				}

				if (!processed)
				{
					if (newCombination == nullptr)
					{
						newCombination = DecoratedCombination::DeriveFrom(cur, dec, socket, decIndex);
					}

					next.push_back(newCombination);
				}
				else
				{
					if (newCombination != nullptr)
					{
						newCombination->Delete();
					}
				}
			};

			// ����ָ� ���� �ʴ� �������̼ǵ� �õ��Ѵ�
			TryToAdd(nullptr, -1, socket);

			int dFrom = cur->lastSocket == socket ? cur->lastDecoratorIndex : 0;

			for (int d = dFrom; d < g_decorators.size(); ++d)
			{
				auto dec = g_decorators[d];

				if (strict)
				{
					if (dec->slotSize != socket + 1) { continue; }
				}
				else
				{
					if (dec->slotSize > socket + 1) { continue; }
				}

				auto si = dec->skillIndex;
				if (cur->skills[si] + 1 <= g_skillMaxLevel[si])
				{
					TryToAdd(dec, d, socket);
				}
			}
#endif
		}

		// �ߺ��� �� �����Ѵ�
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
			socket++;
			first = true;
		}

		WindowsUtility::Debug(
			L"\t%d (passed=%d,evaluated=%d,discarded=%d,equivalent=%d,givenup=%d)\n",
			g_decAll.size(),
			pass, 
			evaluated, 
			rejected[0], 
			rejected[1],
			rejected[2]);
	}
}

//------------------------------------------------------------------------------
void TestDamage()
{
	LoadArmors(false);
	LoadDecorators();
	LoadCharms();
	CheckActiveSkills();
	
	map<Armor::PartType, vector<GeneralizedArmor*>*> g_generalized;
	FilterArmors(g_generalized, false);

	PopulateArmors(g_generalized);

	WeaponDesc weapon;
	MonsterDesc monster;

	// ���� ������ �� �մ´�
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

		//0L"����",
		//1L"����",
		//2L"���� ȸ��",
		//3L"���� Ưȿ",
		//4L"�����Ӽ� ���� ��ȭ",
		//5L"ȭ���� ���",
		//6L"���ź/���ȭ�� ��ȭ",
		//7L"��ź/���� ��ȭ",
		//8L"Ȱ ������ �ܰ� ����",
		//9L"ü��",

		bool arrow =
			(comb->skills[6] == 1 && comb->skills[7] == 1) ||
			(comb->skills[6] == 0 && comb->skills[7] == 0) ||
			g_skills[6] != L"���ź/���ȭ�� ��ȭ" || 
			g_skills[7] != L"��ź/���� ��ȭ";

		bool gambit = 
			(comb->skills[5] >= 2 || comb->skills[5] == 0) || 
			g_skills[5] != L"ȭ���� ���";

		bool move = 
			comb->skills[9] >= 3 || 
			g_skills[9] != L"ü��";

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