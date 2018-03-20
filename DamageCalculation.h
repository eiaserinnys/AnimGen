#pragma once

#include <utility>
#include "Vector.h"

struct CombinationBase;
struct EvaluatingSkills;

//------------------------------------------------------------------------------
struct Desc
{
	std::pair<int, double> attackBonus = std::make_pair(0, 0.0);
	std::pair<int, double> elementalBonus = std::make_pair(0, 0.0);
	double nonElementalBonus = 0;
	double criticalEye = 0;
	double superCritical = 0;
	double exploitWeakness = 0;
	double fireDragonGambit = 0;
	double arrowUpgrade = 0;
	double specialAttack = 0; 	// 특수 사격
	double penetration = 0;		// 관통
	double drawCritical = 0;
	int chargeLevel = 0;

	Desc() = default;

	Desc(const EvaluatingSkills& evSkills, const CombinationBase* comb);
};

//------------------------------------------------------------------------------
struct WeaponDesc
{
	int rarity;

	Core::Vector2D baseDamage;

	double criticalRate;

	int slot1;
	int slot2;
	int slot3;

	bool strongBottle;

	double multiplier;

	bool dragonPiercer;

	WeaponDesc() = default;

	WeaponDesc(
		int rarity, 
		const Core::Vector2D& baseDamage,
		double criticalRate,
		int slot1, int slot2, int slot3,
		bool strongBottle, 
		double multiplier,
		bool dragonPiercer)
		: rarity(rarity)
		, baseDamage(baseDamage)
		, criticalRate(criticalRate)
		, slot1(slot1)
		, slot2(slot2)
		, slot3(slot3)
		, strongBottle(strongBottle)
		, multiplier(multiplier)
		, dragonPiercer(dragonPiercer)
	{
	}

	double MotionValue(const Desc& desc) const 
	{ 
		if (dragonPiercer)
		{
			return desc.chargeLevel >= 3 ? 0.28 : 0.26;
		}
		else
		{
			return desc.chargeLevel >= 3 ? 0.11 : 0.10;
		}
	}
};

//------------------------------------------------------------------------------
struct MonsterDesc
{
	Core::Vector2D weakDefense;
	double weakHitPercent;

	Core::Vector2D normalDefense;

	MonsterDesc() = default;
	MonsterDesc(
		const Core::Vector2D& weak, 
		double weakHitPercent, 
		const Core::Vector2D& normal) 
		: weakDefense(weak)
		, weakHitPercent(weakHitPercent)
		, normalDefense(normal) {}
};

//------------------------------------------------------------------------------
double Calculate(
	FILE* file, 
	const WeaponDesc& weapon, 
	const Desc& desc, 
	const MonsterDesc& monster);

