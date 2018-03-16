#pragma once

#include <utility>
#include "Vector.h"

struct CombinationBase;

//------------------------------------------------------------------------------
struct Desc
{
	std::pair<int, double> attackBonus;
	std::pair<int, double> elementalBonus;
	double criticalEye;
	double superCritical;
	double exploitWeakness;
	double fireDragonGambit;
	double arrowUpgrade;
	int chargeLevel;

	Desc() = default;

	Desc(const CombinationBase* comb);
};

//------------------------------------------------------------------------------
struct WeaponDesc
{
	Core::Vector2D baseDamage;

	double criticalRate;

	int slot1;
	int slot2;
	int slot3;

	double multiplier;

	WeaponDesc() = default;

	WeaponDesc(
		const Core::Vector2D& baseDamage,
		double criticalRate,
		int slot1, int slot2, int slot3,
		double multiplier)
		: baseDamage(baseDamage)
		, criticalRate(criticalRate)
		, slot1(slot1)
		, slot2(slot2)
		, slot3(slot3)
		, multiplier(multiplier)
	{
	}

	double MotionValue(const Desc& desc) const 
	{ 
		return desc.chargeLevel >= 3 ? 0.11 : 0.10; 
	}
};

//------------------------------------------------------------------------------
struct MonsterDesc
{
	double physicalDefense = 0.0;
	double elementalDefense = 0.0;

	MonsterDesc() = default;
	MonsterDesc(double p, double e) : physicalDefense(p), elementalDefense(e) {}
};

//------------------------------------------------------------------------------
double Calculate(
	FILE* file, 
	const WeaponDesc& weapon, 
	const Desc& desc, 
	const MonsterDesc& monster);
