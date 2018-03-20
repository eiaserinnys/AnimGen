#pragma once

struct Decorator;

struct SkillDescriptor
{
	std::wstring name;
	std::wstring abbName;
	int maxLevel;
	Decorator* decorator;
};

struct EvaluatingSkills
{
	std::vector<SkillDescriptor> list;
	std::vector<int> noDecorator;
	std::vector<std::vector<int>> bySlotSize;

	void Update();

	int GetIndex(const std::wstring& skill) const;
};

//------------------------------------------------------------------------------
int MaxAttackSkillLevel();
std::pair<int, double> AttackSkillBonus(int level);

//------------------------------------------------------------------------------
int MaxElementalSkillLevel();
std::pair<int, double> ElementalSkillLevel(int level);

double NonElementalBonus(int level);

double SpecialAttackBonus(int level);

double DrawCriticalBonus(int level);

//------------------------------------------------------------------------------
int MaxCriticalEyeSkillLevel();
double CriticalEye(int level);

//------------------------------------------------------------------------------
int MaxSuperCriticalSkillLevel();
double SuperCritical(int level);

//------------------------------------------------------------------------------
int MaxExploitWeaknessSkillLevel();
double ExploitWeakness(int level);

//------------------------------------------------------------------------------
double FireDragonsGambit();
