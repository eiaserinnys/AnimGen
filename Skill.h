#pragma once

static std::wstring g_skills[] =
{
	L"공격",
	L"간파",
	L"슈퍼 회심",
	L"약점 특효",
	L"불속성 공격 강화",
	L"화룡의 비기",

	// 활 강화
	L"통상탄/통상화살 강화",
	L"산탄/강사 강화",
	L"활 모으기 단계 해제",

	// 유틸리티
	//L"체술",
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
