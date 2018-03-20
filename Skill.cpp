#include "pch.h"
#include "Skill.h"

#include "Decorator.h"

#include <Utility.h>

using namespace std;

//------------------------------------------------------------------------------
void EvaluatingSkills::Update()
{
	bySlotSize.clear();
	bySlotSize.push_back(vector<int>());
	bySlotSize.push_back(vector<int>());
	bySlotSize.push_back(vector<int>());

	noDecorator.clear();

	for (int i = 0; i < list.size(); ++i)
	{
		list[i].decorator = nullptr;

		bool found = false;
		for (int j = 0; j < g_decorators.size(); ++j)
		{
			if (g_decorators[j]->skill == list [i].name)
			{
				found = true;
				list[i].decorator = g_decorators[j];

				bySlotSize[g_decorators[j]->slotSize - 1].push_back(i);
				break;
			}
		}
		if (!found)
		{
			noDecorator.push_back(i);
		}
	}
}

//------------------------------------------------------------------------------
int EvaluatingSkills::GetIndex(const std::wstring& skill) const
{
	for (int i = 0; i < list.size(); ++i)
	{
		if (list[i].name == skill)
		{
			return i;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
int MaxAttackSkillLevel() { return 7; }

pair<int, double> AttackSkillBonus(int level)
{
	int damage[] = { 0, 3, 6, 9, 12, 15, 18, 21, };
	double critical[] = { 0, 0, 0, 0, 0, 0.05, 0.05, 0.05, };
	if (0 <= level && level < COUNT_OF(damage))
	{
		return make_pair(damage[level], critical[level]);
	}
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxElementalSkillLevel() { return 5; }

pair<int, double> ElementalSkillLevel(int level)
{
	int damage[] = { 0, 30, 60, 100, 100, 100, };
	double modifier[] = { 0, 0, 0, 0, 0.05, 0.1, };
	if (0 <= level && level < COUNT_OF(damage))
	{
		return make_pair(damage[level], modifier[level]);
	}
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
double NonElementalBonus(int level)
{
	return level > 0 ? 0.1 : 0;
}

double SpecialAttackBonus(int level)
{
	if (level <= 0) { return 0; }
	if (level == 1) { return 0.1; }
	//if (level >= 2) 
	{ return 0.2; }
}

double DrawCriticalBonus(int level)
{
	if (level <= 0) { return 0; }
	if (level == 1) { return 0.3; }
	if (level == 2) { return 0.6; }
	//if (level >= 3) 
	{ return 1; }
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
	{
		return modifier[level];
	}
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxExploitWeaknessSkillLevel() { return 3; }

double ExploitWeakness(int level)
{
	double modifier[] = { 0, 0.15, 0.3, 0.5, };
	if (0 <= level && level < COUNT_OF(modifier))
	{
		return modifier[level];
	}
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
double FireDragonsGambit() { return 0.35; }
