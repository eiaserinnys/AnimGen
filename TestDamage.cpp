#include "pch.h"

#include <algorithm>

#include <locale.h>
#include <Utility.h>
#include <WindowsUtility.h>

#include "Vector.h"

#include "Skill.h"
#include "Decorator.h"
#include "Armor.h"
#include "GeneralizedCombination.h"

using namespace std;
//using namespace Core;

static FILE* file = nullptr;

static vector<Set*> g_sets;

static map<Armor::PartType, vector<Armor*>*> g_armors;

static map<Armor::PartType, vector<GeneralizedArmor*>*> g_generalized;

std::vector<Decorator*> g_decorators;

std::vector<Decorator*> g_skillToDecorator;

Core::Vector2D WeaponValue() { return Core::Vector2D(240, 390); }
double WeaponCritical() { return -0.2; }
int WeaponSlot(int slot)
{
	int slots[] = { 0, 0, 0 };
	return slots[slot];
}

double WeaponMultipler() { return 1.2; }

double BaseCriticalDamageRate(double criticalRate) { return criticalRate >= 0 ? 1.25 : 0.75; }
double MotionValue(int chargeLevel) { return chargeLevel >= 3 ? 0.11 : 0.10; }

double PhysicalDefense() { return 0.45; }
double ElementalDefense() { return 0.25; }

//------------------------------------------------------------------------------
struct Desc
{
	pair<int, double> attackBonus;
	pair<int, double> elementalBonus;
	double criticalEye;
	double superCritical;
	double exploitWeakness;
	double fireDragonGambit;
	double arrowUpgrade;
	int chargeLevel;
};

//------------------------------------------------------------------------------
void Calculate(const Desc& desc)
{
	// 무기 기본 대미지
	Core::Vector2D rawDamage = WeaponValue();
	rawDamage.x /= WeaponMultipler();
	rawDamage.y /= 10;

	// 대미지 보너스
	Core::Vector2D rawDamageWithBonus =
		rawDamage +
		Core::Vector2D(
			desc.attackBonus.first, 
			desc.elementalBonus.first / 10);

	if (rawDamageWithBonus.y >= rawDamage.y * 1.3)
	{
		rawDamageWithBonus.y = rawDamage.y * 1.3;
	}

	// 기본 대미지 * 강격병 * 거리 크리티컬
	Core::Vector2D modifiedBaseDamage(
		rawDamageWithBonus.x * (1 + desc.arrowUpgrade) * 1.5 * 1.5,
		rawDamageWithBonus.y * (1 + desc.elementalBonus.second));

	// 회심률
	auto criticalProbability =
		WeaponCritical() +
		desc.attackBonus.second +
		desc.criticalEye +
		desc.exploitWeakness;

	if (criticalProbability > 1) { criticalProbability = 1; }
	if (criticalProbability < - 1) { criticalProbability = -1; }

	double absCriticalProb = abs(criticalProbability);

	// 물리 회심 배율
	auto physicalCriticalRate =
		BaseCriticalDamageRate(criticalProbability) +
		desc.superCritical;

	// 속성 회심 배율
	auto elementalCriticalRate =
		1 +
		desc.fireDragonGambit;

	// 기대 대미지
	//Core::Vector2D expectedDamage(
	//	modifiedBaseDamage.x * criticalProbability * physicalCriticalRate +
	//	modifiedBaseDamage.x * (1 - criticalProbability),
	//	modifiedBaseDamage.y * criticalProbability * elementalCriticalRate +
	//	modifiedBaseDamage.y * (1 - criticalProbability)
	//);
	Core::Vector2D expectedDamage(
		modifiedBaseDamage.x, 
		modifiedBaseDamage.y);

	Core::Vector2D criticalExpectedDamage(
		modifiedBaseDamage.x * physicalCriticalRate,
		modifiedBaseDamage.y * elementalCriticalRate);

	// 모션 적용
	Core::Vector2D motionDamage =
		expectedDamage *
		Core::Vector2D(MotionValue(desc.chargeLevel), 1);
	Core::Vector2D criticalMotionDamage =
		criticalExpectedDamage * 
		Core::Vector2D(MotionValue(desc.chargeLevel), 1);

	// 방어력 적용
	Core::Vector2D appliedDamage =
		motionDamage *
		Core::Vector2D(PhysicalDefense(), ElementalDefense());
	Core::Vector2D criticalAppliedDamage =
		criticalMotionDamage *
		Core::Vector2D(PhysicalDefense(), ElementalDefense());

	// 최종 기대 대미지
	Core::Vector2D finalExpectedDamage =
		Core::Vector2D(
			int(appliedDamage.x) * (1 - absCriticalProb) +
			int(criticalAppliedDamage.x) * absCriticalProb,
			int(appliedDamage.y) * (1 - absCriticalProb) +
			int(criticalAppliedDamage.y) * absCriticalProb);

	fwprintf(
		file,
		L"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"

		"%.3f\t%.3f\t%.3f\t"

		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"

		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"

		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"

		"%.3f\t%.3f\t"
		"%.3f\t",
		rawDamageWithBonus.x, rawDamageWithBonus.y,
		modifiedBaseDamage.x, modifiedBaseDamage.y,

		criticalProbability,
		physicalCriticalRate, elementalCriticalRate, 

		expectedDamage.x, expectedDamage.y,
		criticalExpectedDamage.x, criticalExpectedDamage.y,

		motionDamage.x, motionDamage.y,
		criticalMotionDamage.x, criticalMotionDamage.y,

		appliedDamage.x, appliedDamage.y,
		criticalAppliedDamage.x, criticalAppliedDamage.y,

		finalExpectedDamage.x, finalExpectedDamage.y,
		finalExpectedDamage.x + finalExpectedDamage.y);
}

//------------------------------------------------------------------------------
vector<wstring> Tokenize(const wstring& s_)
{
	vector<wstring> result;

	wstring s = s_;

	size_t pos = 0;
	wstring token;
	while ((pos = s.find(L',')) != wstring::npos) 
	{
		token = s.substr(0, pos);
		result.push_back(token);
		s.erase(0, pos + 1);
	}
	result.push_back(s);

	return result;
}

//------------------------------------------------------------------------------
struct ArmorInstance
{
	Armor* original = nullptr;
	Decorator* decorators[3];

	ArmorInstance()
	{
		memset(decorators, 0, sizeof(Decorator*) * COUNT_OF(decorators));
	}
};

//------------------------------------------------------------------------------
void LoadArmors()
{
	fopen_s(&file, "ArmorData", "r,ccs=UTF-8");

	Set* curSet = nullptr;
	int offset = 0;

	g_armors.insert(make_pair(Armor::Head, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Body, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Arm, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Waist, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Leg, new vector<Armor*>));

	wchar_t buffer[1024];
	while (fgetws(&buffer[0], 1024, file) != nullptr)
	{
		// 먼저 뉴라인 문자를 제거한다
		wstring str = buffer;
		size_t pos;
		if ((pos = str.find(L'\n')) != wstring::npos)
		{ 
			str = str.substr(0, pos);
		}

		// 쉼표로 자른다
		auto tokens = Tokenize(str);

		if (offset == 0)
		{
			curSet = new Set;
			curSet->name = tokens[0];
			curSet->rarity = _wtoi(tokens[1].c_str());
			curSet->defense = _wtoi(tokens[2].c_str());
			if (tokens.size() >= 7) { curSet->skill = tokens[6].c_str(); }
			g_sets.push_back(curSet);
		}
		else if (offset < 6)
		{
			if (tokens.size() >= 3)
			{
				auto part = new Armor;
				part->type = (Armor::PartType) (offset - 1);
				part->set = curSet;
				part->name = tokens[0];

				int skillCount = _wtoi(tokens[1].c_str());
				for (int i = 0; i < skillCount; ++i)
				{
					part->skills.push_back(make_pair(
						tokens[2 + i * 2 + 0],
						_wtoi(tokens[2 + i * 2 + 1].c_str())));
				}

				int slotCount = _wtoi(tokens[2 + skillCount * 2].c_str());
				for (int i = 0; i < slotCount; ++i)
				{
					part->slots.push_back(
						_wtoi(tokens[3 + skillCount * 2 + i].c_str()));
				}

				g_armors[part->type]->push_back(part);

#if 0
				WindowsUtility::Debug(L"\"%s\"\t", part->name.c_str());
				for (int i = 0; i < part->skills.size(); ++i)
				{
					WindowsUtility::Debug(
						L"%s Lv.%d\t", 
						part->skills[i].first.c_str(),
						part->skills[i].second);
				}
				if (!part->set->skill.empty())
				{
					WindowsUtility::Debug(
						L"%s\t",
						part->set->skill.c_str());
				}
				for (int i = 0; i < part->slots.size(); ++i)
				{
					WindowsUtility::Debug(L"[%d] ", part->slots[i]);
				}
				WindowsUtility::Debug(L"\n");
#endif
			}
		}

		offset = (offset + 1) % 7;
	}

	fclose(file);
}

//------------------------------------------------------------------------------
int GetSkillIndex(const wstring& skill)
{
	for (int i = 0; i < COUNT_OF(g_skills); ++i)
	{
		if (g_skills[i] == skill)
		{
			return i;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
void LoadDecorators()
{
	fopen_s(&file, "DecorationData", "r,ccs=UTF-8");

	wchar_t buffer[1024];
	while (fgetws(&buffer[0], 1024, file) != nullptr)
	{
		// 먼저 뉴라인 문자를 제거한다
		wstring str = buffer;
		size_t pos;
		if ((pos = str.find(L'\n')) != wstring::npos)
		{
			str = str.substr(0, pos);
		}

		// 쉼표로 자른다
		auto tokens = Tokenize(str);

		if (tokens.size() >= 4)
		{
			int index = GetSkillIndex(tokens[0]);
			if (index >= 0)
			{
				auto dec = new Decorator;
				dec->name = tokens[3];
				dec->skill = tokens[0];
				dec->skillIndex = index;
				dec->rarity = _wtoi(tokens[2].c_str());
				dec->slotSize = _wtoi(tokens[1].c_str());

				g_decorators.push_back(dec);
			}

			//WindowsUtility::Debug(L"%s %s R%d [%d]\n", dec->name.c_str(), dec->skill.c_str(), dec->rarity, dec->slotSize);
		}
	}

	fclose(file);
}

//------------------------------------------------------------------------------
void FilterArmors()
{
	g_generalized.insert(make_pair(Armor::Head, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Body, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Arm, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Waist, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Leg, new vector<GeneralizedArmor*>));

	int total = 1;

	for (int i = 0; i < Armor::Count; ++i)
	{
		auto parts = g_armors[(Armor::PartType) i];
		for (int j = 0; j < parts->size(); ++j)
		{
			auto part = (*parts)[j];

			GeneralizedArmor general(COUNT_OF(g_skills));

			// 셋트 스킬이 포함시킬 스킬인 경우
			int index = GetSkillIndex(part->set->skill);
			if (index >= 0) { general.skills[index]++; }

			// 개별 파트 스킬이 포함시킬 스킬인 경우
			for (int k = 0; k < part->skills.size(); ++k)
			{
				int index = GetSkillIndex(part->skills[k].first);
				if (index >= 0)
				{
					general.skills[index] += part->skills[k].second;
				}
			}

			// 슬롯을 적용한다
			for (int k = 0; k < part->slots.size(); ++k)
			{
				general.slots[part->slots[k] - 1]++;
			}

			// scan
			GeneralizedArmor::ComparisonResult compare = GeneralizedArmor::NotWorse;
			for (int k = 0; k < g_generalized[(Armor::PartType) i]->size(); ++k)
			{
				auto prev = (*g_generalized[(Armor::PartType) i])[k];

				compare = general.Compare(*prev);

				if (compare == GeneralizedArmor::Equal)
				{
					prev->source.push_back(part);
					break;
				}
				else if (compare == GeneralizedArmor::Worse)
				{
					break;
				}
			}

			bool added = false;
			if (compare == GeneralizedArmor::NotWorse)
			{
				auto newGeneral = new GeneralizedArmor(COUNT_OF(g_skills));
				*newGeneral = general;

				newGeneral->source.push_back(part);

				g_generalized[(Armor::PartType) i]->push_back(newGeneral);

				added = true;
			}
		}

		for (int j = 0; j < g_generalized[(Armor::PartType) i]->size(); )
		{
			// 다른 조합보다 불리한 조합은 모두 제거한다
			auto* cur = (*g_generalized[(Armor::PartType) i])[j];

			bool bad = false;

			for (int k = j + 1; k < g_generalized[(Armor::PartType) i]->size(); )
			{
				auto* rhs = (*g_generalized[(Armor::PartType) i])[k];

				if (cur->IsWorseThanOrEqualTo(*rhs))
				{
					WindowsUtility::Debug(L"Dropping '");
					cur->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					rhs->DumpSimple();
					WindowsUtility::Debug(L"'.\n");

					bad = true;
					break;
				}
				else if (rhs->IsWorseThanOrEqualTo(*cur))
				{
					WindowsUtility::Debug(L"Dropping '");
					rhs->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					cur->DumpSimple();
					WindowsUtility::Debug(L"'.\n");

					g_generalized[(Armor::PartType) i]->erase(
						g_generalized[(Armor::PartType) i]->begin() + k);
					delete rhs;
				}
				else
				{
					k++;
				}
			}

			if (bad)
			{
				g_generalized[(Armor::PartType) i]->erase(
					g_generalized[(Armor::PartType) i]->begin() + j);
				delete cur;
			}
			else
			{
				++j;
			}
		}

		for (int j = 0; j < g_generalized[(Armor::PartType) i]->size(); j++)
		{
			auto* general = (*g_generalized[(Armor::PartType) i])[j];

			general->Dump();
		}

		int validParts = g_generalized[(Armor::PartType) i]->size();
		WindowsUtility::Debug(L"%d-------------------------\n", validParts);
		total *= validParts;
	}

	WindowsUtility::Debug(L"Total Combination = %d\n", total);
}

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

			auto result3 = GeneralizedCombinationBase::Compare(*toEvaluate, *target);

#if 0
			{
				auto result = toEvaluate->Compare(*target);
				auto result2 = target->Compare(*toEvaluate);

				if (result == GeneralizedCombinationBase::Equal)
				{
					assert(result2 == GeneralizedCombinationBase::Equal);
					assert(result3 == GeneralizedCombinationBase::Equal);
				}
				else if (result == GeneralizedCombinationBase::Worse)
				{
					assert(result2 == GeneralizedCombinationBase::NotWorse);
					assert(result3 == GeneralizedCombinationBase::Worse);
				}
				else if (result == GeneralizedCombinationBase::NotWorse)
				{
					if (result2 == GeneralizedCombinationBase::NotWorse)
					{
						result3 = GeneralizedCombinationBase::Compare(*toEvaluate, *target);
						assert(result3 == GeneralizedCombinationBase::Undetermined);
					}
					else if (result2 == GeneralizedCombinationBase::Worse)
					{
						assert(result3 == GeneralizedCombinationBase::Better);
					}
					else
					{
						auto result2 = target->Compare(*toEvaluate);
						assert(!"impossible");
					}
				}
			}
#endif

			if (result3 == GeneralizedCombinationBase::Equal)
			{
				// 우측을 좌측에 더하자
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
			else if (result3 == GeneralizedCombinationBase::Worse)
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
			else if (result3 == GeneralizedCombinationBase::Better)
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
void PopulateArmors()
{
	// 초기 리스트를 만든다
	{
		int channel = 0;
		auto& gs = *g_generalized[(Armor::PartType) channel];

		for (int i = 0; i < gs.size(); ++i)
		{
			auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
			newComb->Combine(nullptr, channel, gs[i]);
			g_all.push_back(newComb);
		}

		WindowsUtility::Debug(L"%d\n", g_all.size());

		// 전체 덤프 ㄱㄱ
		for (auto it = g_all.begin(); it != g_all.end(); ++it)
		{
			(*it)->Dump();
		}
	}

	// 다음 채널을 추가해서 늘린다
	for (int channel = 1; channel < Armor::Count; ++channel)
	{
		list<GeneralizedCombination*> next;

		auto& gs = *g_generalized[(Armor::PartType) channel];

		// 모든 조합 x 조합의 페어를 일단 생성
		for (auto it = g_all.begin(); it != g_all.end(); ++it)
		{
			for (int i = 0; i < gs.size(); ++i)
			{
				auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
				newComb->Combine(*it, channel, gs[i]);
				next.push_back(newComb);
			}
		}

		// 전체 덤프 ㄱㄱ
		int rejected = RejectWorseCombinations(next, false);

		for (auto it = g_all.begin(); it != g_all.end(); ++it)
		{
			delete *it;
		}

		g_all.swap(next);

		WindowsUtility::Debug(L"%d (%d)---------------------------------------\n", g_all.size(), rejected);
	}

	//for (auto it = g_all.begin(); it != g_all.end(); ++it) { (*it)->Dump(); }
}

//------------------------------------------------------------------------------
bool AddIfBetter(
	list<DecoratedCombination*>& next, 
	DecoratedCombination* newCom, 
	int& rejected)
{
	for (auto it = next.begin(); it != next.end(); ++it)
	{
		auto toCompare = *it;

		auto ret = DecoratedCombination::Compare(*newCom, *toCompare);

		if (ret == DecoratedCombination::Better)
		{
			newCom->equivalents.push_back(toCompare);
			*it = newCom;
			++rejected;		// 옛날 게 리젝트됨
			return true;
		}
		else if (ret == DecoratedCombination::Equal)
		{
			toCompare->equivalents.push_back(newCom);
			newCom->addedAsEquivalent = true;
			rejected++;
			return false;
		}
		else if (ret == DecoratedCombination::Worse)
		{
			newCom->Delete();
			rejected++;
			return false;
		}
	}

	next.push_back(newCom);
	return true;
}

//------------------------------------------------------------------------------
list<DecoratedCombination*> g_decAll;

void PopulateDecorators()
{
	// 데커레이티드 컴비네이션을 새로 만든다
	for (auto comb : g_all)
	{
		g_decAll.push_back(DecoratedCombination::DeriveFrom(comb));
	}

	// 작은 소켓부터 채워가며 리스트를 구축해보자
	// 소켓 크기에 대해서 순회하는 것에 주의
	for (int socket = 2; socket >= 0; --socket)
	{
		bool needToIterate = false;

		WindowsUtility::Debug(L"Filling socket with size %d\n", socket + 1);
		do
		{
			list<DecoratedCombination*> next;
			int rejected0 = 0, rejected1 = 0;

			for (auto it = g_decAll.begin(); it != g_decAll.end(); )
			{
				auto cur = *it;

				if (cur->slots[socket] > 0)
				{
					for (int d = 0; d < g_decorators.size(); ++d)
					{
						auto dec = g_decorators[d];
						if (dec->slotSize > socket + 1) { continue; }

						auto nextCom = DecoratedCombination::DeriveFrom(cur);

						bool notMaxed = cur->skills[dec->skillIndex] + 1 <= g_skillMaxLevel[dec->skillIndex];

						nextCom->skills[dec->skillIndex] = 
							notMaxed ? 
								cur->skills[dec->skillIndex] + 1 : 
								g_skillMaxLevel[dec->skillIndex];
						nextCom->slots[socket]--;
						nextCom->decorator = notMaxed ? dec : nullptr;

						AddIfBetter(next, nextCom, rejected0);
					}

					++it;
				}
				else
				{
					AddIfBetter(next, cur, rejected0);
					it = g_decAll.erase(it);
				}
			}

			// 중복된 페어를 제거한다
			rejected1 = RejectWorseCombinations(next, false);

			for (auto inst : g_decAll) { inst->Delete(); }

			g_decAll.swap(next);

			needToIterate = false;
			for (auto inst : g_decAll)
			{
				needToIterate = inst->slots[socket] > 0;
				if (needToIterate) { break; }
			}

			WindowsUtility::Debug(
				L"\t%d (%d,%d)-----------\n",
				g_decAll.size(),
				rejected0, 
				rejected1);
		}
		while (needToIterate);
	}
}

//------------------------------------------------------------------------------
void TestDamage()
{
	LoadArmors();
	LoadDecorators();

	g_skillToDecorator.resize(COUNT_OF(g_skills), nullptr);

	for (int i = 0; i < COUNT_OF(g_skills); ++i)
	{
		for (int j = 0; j < g_decorators.size(); ++j)
		{
			if (g_decorators[j]->skill == g_skills[i])
			{
				g_skillToDecorator[i] = g_decorators[j];
				break;
			}
		}
	}
	
	FilterArmors();

	PopulateArmors();

	// 무기 1슬롯을 더 뚫는다
	for (auto it = g_all.begin(); it != g_all.end(); ++it)
	{
		auto comb = *it;
		comb->slots[0] += WeaponSlot(0);
		comb->slots[1] += WeaponSlot(1);
		comb->slots[2] += WeaponSlot(2);
	}

	PopulateDecorators();

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
			(comb->skills[6] == 0 && comb->skills[7] == 0);

		bool gambit = comb->skills[5] >= 2 || comb->skills[5] >= 0;

		if (arrow && gambit)
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
				L"%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",
				comb->skills[0],
				comb->skills[1],
				comb->skills[2],
				comb->skills[3],
				comb->skills[4],
				comb->skills[5],
				comb->skills[6],
				comb->skills[7],
				comb->skills[8]);

			Calculate(desc);

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