#pragma once

#include <utility>
#include "Vector.h"

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
};

//------------------------------------------------------------------------------
struct WeaponDesc
{
	Core::Vector2D baseDamage = Core::Vector2D(180, 390);

	double criticalRate = 0;

	int slots[3] = { 0, 0, 0, };

	double multiplier = 1.2;

	double MotionValue(const Desc& desc) const 
	{ 
		return desc.chargeLevel >= 3 ? 0.11 : 0.10; 
	}
};

//------------------------------------------------------------------------------
struct MonsterDesc
{
	double physicalDefense = 0.6;
	double elementalDefense = 0.4;
};

//------------------------------------------------------------------------------
void Calculate(
	FILE* file, 
	const WeaponDesc& weapon, 
	const Desc& desc, 
	const MonsterDesc& monster);
