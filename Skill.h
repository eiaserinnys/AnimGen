#pragma once

static std::wstring g_skills[] =
{
	L"����",
	L"����",
	L"���� ȸ��",
	L"���� Ưȿ",
	L"�����Ӽ� ���� ��ȭ",
	L"ȭ���� ���",

	// Ȱ ��ȭ
	L"���ź/���ȭ�� ��ȭ",
	L"��ź/���� ��ȭ",
	L"Ȱ ������ �ܰ� ����",

	// ��ƿ��Ƽ
	L"ü��",
};

static std::wstring g_skillsAbb[] =
{
	L"����",
	L"����",
	L"��ȸ",
	L"��Ư",
	L"�Ӱ�",
	L"ȭ��",

	L"�밭",
	L"�갭",
	L"Ȱ��",

	L"ü��"
};

static int g_skillMaxLevel[] =
{
	7,
	7,
	3, 
	3, 
	5,
	2, 
	1,
	1,
	1,
	5,
};

struct Decorator;

extern std::vector<Decorator*> g_skillToDecorator;

extern std::vector<int> g_skillWithNoDecorator;

extern std::vector<std::vector<int>> g_skillsBySlotSize;

void CheckActiveSkills();

//------------------------------------------------------------------------------
int GetSkillIndex(const std::wstring& skill);

//------------------------------------------------------------------------------
int MaxAttackSkillLevel();
std::pair<int, double> AttackSkillBonus(int level);

//------------------------------------------------------------------------------
int MaxElementalSkillLevel();
std::pair<int, double> ElementalSkillLevel(int level);

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
