#include "pch.h"
#include "DamageCalculation.h"

#include "Vector.h"

//------------------------------------------------------------------------------
inline double BaseCriticalDamageRate(double criticalRate) 
{ 
	return criticalRate >= 0 ? 1.25 : 0.75; 
}

//------------------------------------------------------------------------------
void Calculate(
	FILE* file, 
	const WeaponDesc& weapon, 
	const Desc& desc,
	const MonsterDesc& monster)
{
	// 무기 기본 대미지
	Core::Vector2D rawDamage = weapon.baseDamage;
	rawDamage.x /= weapon.multiplier;
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
		weapon.criticalRate +
		desc.attackBonus.second +
		desc.criticalEye +
		desc.exploitWeakness;

	if (criticalProbability > 1) { criticalProbability = 1; }
	if (criticalProbability < -1) { criticalProbability = -1; }

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
		Core::Vector2D(weapon.MotionValue(desc), 1);
	Core::Vector2D criticalMotionDamage =
		criticalExpectedDamage *
		Core::Vector2D(weapon.MotionValue(desc), 1);

	// 방어력 적용
	Core::Vector2D appliedDamage =
		motionDamage *
		Core::Vector2D(monster.physicalDefense, monster.elementalDefense);
	Core::Vector2D criticalAppliedDamage =
		criticalMotionDamage *
		Core::Vector2D(monster.physicalDefense, monster.elementalDefense);

	// 최종 기대 대미지
	Core::Vector2D finalExpectedDamage =
		Core::Vector2D(
			int(appliedDamage.x) * (1 - absCriticalProb) +
			int(criticalAppliedDamage.x) * absCriticalProb,
			int(appliedDamage.y) * (1 - absCriticalProb) +
			int(criticalAppliedDamage.y) * absCriticalProb);

	if (file != nullptr)
	{
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
}
