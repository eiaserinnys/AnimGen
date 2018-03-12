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

Core::Vector2D WeaponValue() { return Core::Vector2D(204, 270); }
double WeaponMultipler() { return 1.2; }
double WeaponCritical() { return 0.15; }
double BaseCriticalDamageRate() { return 1.25; }
double MotionValue() { return 0.11; }

double PhysicalDefense() { return 0.45; }
double ElementalDefense() { return 0.2; }

//------------------------------------------------------------------------------
int MaxAttackSkillLevel() { return 7; }

pair<int, double> AttackSkillBonus(int level)
{
	int damage[] = { 0, 3, 6, 9, 12, 15, 18, 21, };
	double critical[] = { 0, 0, 0, 0, 0, 0.05, 0.05, 0.05, };
	if (0 <= level && level < COUNT_OF(damage)) 
	{ return make_pair(damage[level], critical[level]); }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxElementalSkillLevel() { return 5; }

pair<int, double> ElementalSkillLevel(int level)
{
	int damage[] = { 0, 30, 60, 100, 100, 100, };
	double modifier[] = { 0, 0, 0, 0, 0.05, 0.1, };
	if (0 <= level && level < COUNT_OF(damage))
	{ return make_pair(damage[level], modifier[level]); }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxCriticalEyeSkillLevel() { return 7; }

double CriticalEye(int level)
{
	double modifier[] = { 0, 0.03, 0.06, 0.1, 0.15, 0.2, 0.25, 0.3 };
	if (0 <= level && level < COUNT_OF(modifier)) { return modifier[level]; }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxSuperCriticalSkillLevel() { return 3; }

double SuperCritical(int level)
{
	double modifier[] = { 0, 0.05, 0.1, 0.15, };
	if (0 <= level && level < COUNT_OF(modifier))
	{ return modifier[level]; }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxExploitWeaknessSkillLevel() { return 3; }

double ExploitWeakness(int level)
{
	double modifier[] = { 0, 0.15, 0.3, 0.5, };
	if (0 <= level && level < COUNT_OF(modifier))
	{ return modifier[level]; }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
double FireDragonsGambit() { return 0.1; }

//------------------------------------------------------------------------------
struct Desc
{
	pair<int, double> attackBonus;
	pair<int, double> elementalBonus;
	double criticalEye;
	double superCritical;
	double exploitWeakness;
	double fireDragonGambit;
};

//------------------------------------------------------------------------------
void Calculate(const Desc& desc)
{
	// ���� �⺻ �����
	Core::Vector2D rawDamage = WeaponValue();
	rawDamage.x /= WeaponMultipler();
	rawDamage.y /= 10;

	// ����� ���ʽ�
	Core::Vector2D rawDamageWithBonus =
		rawDamage +
		Core::Vector2D(
			desc.attackBonus.first, 
			desc.elementalBonus.first);

	// �⺻ ����� * ���ݺ� * �Ÿ� ũ��Ƽ��
	Core::Vector2D modifiedBaseDamage(
		rawDamageWithBonus.x * 1.5 * 1.5,
		rawDamageWithBonus.y * 1.5 * 1.5 * (1 + desc.elementalBonus.second));

	// ȸ�ɷ�
	auto criticalProbability =
		WeaponCritical() +
		desc.attackBonus.second +
		desc.criticalEye +
		desc.exploitWeakness;

	if (criticalProbability > 1) { criticalProbability = 1; }

	// ���� ȸ�� ����
	auto physicalCriticalRate =
		BaseCriticalDamageRate() +
		desc.superCritical;

	// �Ӽ� ȸ�� ����
	auto elementalCriticalRate =
		1 +
		desc.fireDragonGambit;

	// ��� �����
	Core::Vector2D criticalExpectedDamage(
		modifiedBaseDamage.x * criticalProbability * physicalCriticalRate +
		modifiedBaseDamage.x * (1 - criticalProbability),
		modifiedBaseDamage.y * criticalProbability * elementalCriticalRate +
		modifiedBaseDamage.y * (1 - criticalProbability)
	);

	// ��� ����
	Core::Vector2D motionDamage = criticalExpectedDamage * MotionValue();

	// ���� ����
	Core::Vector2D finalDamage =
		motionDamage *
		Core::Vector2D(PhysicalDefense(), ElementalDefense());

	fprintf(
		file,
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t"
		"%d"
		"\n",
		rawDamageWithBonus.x, rawDamageWithBonus.y,
		modifiedBaseDamage.x, modifiedBaseDamage.y,
		criticalProbability, physicalCriticalRate, elementalCriticalRate,
		criticalExpectedDamage.x, criticalExpectedDamage.y,
		motionDamage.x, motionDamage.y,
		finalDamage.x, finalDamage.y,
		finalDamage.x + finalDamage.y, 
		(int) (finalDamage.x + finalDamage.y));
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
		// ���� ������ ���ڸ� �����Ѵ�
		wstring str = buffer;
		size_t pos;
		if ((pos = str.find(L'\n')) != wstring::npos)
		{ 
			str = str.substr(0, pos);
		}

		// ��ǥ�� �ڸ���
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
		// ���� ������ ���ڸ� �����Ѵ�
		wstring str = buffer;
		size_t pos;
		if ((pos = str.find(L'\n')) != wstring::npos)
		{
			str = str.substr(0, pos);
		}

		// ��ǥ�� �ڸ���
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

			// ��Ʈ ��ų�� ���Խ�ų ��ų�� ���
			int index = GetSkillIndex(part->set->skill);
			if (index >= 0) { general.skills[index]++; }

			// ���� ��Ʈ ��ų�� ���Խ�ų ��ų�� ���
			for (int k = 0; k < part->skills.size(); ++k)
			{
				int index = GetSkillIndex(part->skills[k].first);
				if (index >= 0)
				{
					general.skills[index] += part->skills[k].second;
				}
			}

			// ������ �����Ѵ�
			for (int k = 0; k < part->slots.size(); ++k)
			{
				general.slots[part->slots[k] - 1]++;
			}

			// scan
			bool duplication = false;
			for (int k = 0; k < g_generalized[(Armor::PartType) i]->size(); ++k)
			{
				auto prev = (*g_generalized[(Armor::PartType) i])[k];

				if (*prev == general)
				{
					prev->source.push_back(part);
					duplication = true;
					break;
				}
			}

			bool added = false;
			if (!duplication)
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
			// �ٸ� ���պ��� �Ҹ��� ������ ��� �����Ѵ�
			auto* cur = (*g_generalized[(Armor::PartType) i])[j];

			bool bad = false;

			for (int k = j + 1; k < g_generalized[(Armor::PartType) i]->size(); )
			{
				auto* rhs = (*g_generalized[(Armor::PartType) i])[k];

				if (*cur <= *rhs)
				{
					WindowsUtility::Debug(L"Dropping '");
					cur->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					rhs->DumpSimple();
					WindowsUtility::Debug(L"'.\n");

					bad = true;
					break;
				}
				else if (*rhs <= *cur)
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
void PopulateArmors()
{
	// �ʱ� ����Ʈ�� �����
	list<GeneralizedCombination*> all;

	{
		int channel = 0;
		auto& gs = *g_generalized[(Armor::PartType) channel];

		for (int i = 0; i < gs.size(); ++i)
		{
			auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
			newComb->Combine(*gs[i]);
			all.push_back(newComb);
		}

		WindowsUtility::Debug(L"%d\n", all.size());
	}

	// ���� ä���� �߰��ؼ� �ø���
	for (int channel = 1; channel < Armor::Count; ++channel)
	{
		list<GeneralizedCombination*> next;

		auto& gs = *g_generalized[(Armor::PartType) channel];

		// ��� ���� x ������ �� �ϴ� ����
		for (auto it = all.begin(); it != all.end(); ++it)
		{
			for (int i = 0; i < gs.size(); ++i)
			{
				auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
				*newComb = **it;
				newComb->Combine(*gs[i]);
				next.push_back(newComb);
			}
		}

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
				if (*toEvaluate <= *target)
				{
#if 0
					WindowsUtility::Debug(L"Dropping '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					target->DumpSimple();
					WindowsUtility::Debug(L"'.\n");
#endif

					bad = true;
					break;
				}
				else if (*target <= *toEvaluate)
				{
#if 0
					WindowsUtility::Debug(L"Dropping '");
					target->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"'.\n");
#endif

					++rejected;
					delete target;
					jt = next.erase(jt);
				}
				else
				{
					jt++;
				}
			}

			if (bad)
			{
				++rejected;
				delete toEvaluate;
				it = next.erase(it);
			}
			else
			{
				it++;
			}
		}

		for (auto it = all.begin(); it != all.end(); ++it)
		{
			delete *it;
		}

		all.swap(next);

		WindowsUtility::Debug(L"%d (%d)---------------------------------------\n", all.size(), rejected);
	}

	for (auto it = all.begin(); it != all.end(); ++it)
	{
		(*it)->Dump();
	}

#if 0
	list<GeneralizedCombination*> skillOnly;
	list<GeneralizedCombination*> slotOnly;

	for (auto it = all.begin(); it != all.end(); ++it)
	{
		{
			auto s = new GeneralizedCombination(**it);

			memset(s->skills, 0, sizeof(int) * s->skillCount);

			bool duplicated = false;
			for (auto sit = slotOnly.begin(); sit != slotOnly.end(); ++sit)
			{
				if (**sit == *s)
				{
					duplicated = true;
					delete s;
					break;
				}
			}

			if (!duplicated) { slotOnly.push_back(s); }
		}

		{
			auto s = new GeneralizedCombination(**it);

			memset(s->slots, 0, sizeof(int) * COUNT_OF(s->slots));

			bool duplicated = false;
			for (auto sit = skillOnly.begin(); sit != skillOnly.end(); ++sit)
			{
				if (**sit == *s)
				{
					duplicated = true;
					delete s;
					break;
				}
			}

			if (!duplicated) { skillOnly.push_back(s); }
		}
	}

	for (auto it = skillOnly.begin(); it != skillOnly.end(); ++it)
	{
		auto s = *it;
		s->Dump();
	}

	for (auto it = slotOnly.begin(); it != slotOnly.end(); ++it)
	{
		auto s = *it;
		s->Dump();
	}
#endif
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

	fopen_s(&file, "log_damage.txt", "w");

	Desc desc;

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

	fclose(file);
}