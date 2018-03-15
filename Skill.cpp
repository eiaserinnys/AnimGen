#include "pch.h"
#include "Skill.h"

#include "Decorator.h"

#include <Utility.h>

using namespace std;

vector<Decorator*> g_skillToDecorator;

vector<int> g_skillWithNoDecorator;

vector<vector<int>> g_skillsBySlotSize;

//------------------------------------------------------------------------------
void CheckActiveSkills()
{
	g_skillToDecorator.resize(COUNT_OF(g_skills), nullptr);

	g_skillsBySlotSize.push_back(vector<int>());
	g_skillsBySlotSize.push_back(vector<int>());
	g_skillsBySlotSize.push_back(vector<int>());

	for (int i = 0; i < COUNT_OF(g_skills); ++i)
	{
		bool found = false;
		for (int j = 0; j < g_decorators.size(); ++j)
		{
			if (g_decorators[j]->skill == g_skills[i])
			{
				found = true;
				g_skillToDecorator[i] = g_decorators[j];

				g_skillsBySlotSize[g_decorators[j]->slotSize - 1].push_back(i);
				break;
			}
		}
		if (!found)
		{
			g_skillWithNoDecorator.push_back(i);
		}
	}
}

//------------------------------------------------------------------------------
int GetSkillIndex(const std::wstring& skill)
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
