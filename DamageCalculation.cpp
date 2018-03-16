#include "pch.h"
#include "DamageCalculation.h"

#include "Vector.h"

#include "Skill.h"
#include "CombinationBase.h"

//------------------------------------------------------------------------------
Desc::Desc(const CombinationBase* comb)
{
	attackBonus = AttackSkillBonus(comb->skills[0]);
	criticalEye = CriticalEye(comb->skills[1]);
	superCritical = SuperCritical(comb->skills[2]);
	exploitWeakness = ExploitWeakness(comb->skills[3]);
	elementalBonus = ElementalSkillLevel(comb->skills[4]);
	fireDragonGambit = comb->skills[5] >= 2 ? FireDragonsGambit() : 0;
	arrowUpgrade = comb->skills[6] >= 1 && comb->skills[7] >= 1 ? 0.1 : 0;
	chargeLevel = comb->skills[8] >= 1 ? 3 : 2;
}

//------------------------------------------------------------------------------
inline double BaseCriticalDamageRate(double criticalRate) 
{ 
	return criticalRate >= 0 ? 1.25 : 0.75; 
}

//------------------------------------------------------------------------------
double Calculate(
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

	// 회심률 (비약점, 약점)
	auto criticalProbability = Core::Vector2D(
		weapon.criticalRate + desc.attackBonus.second + desc.criticalEye, 
		weapon.criticalRate + desc.attackBonus.second + desc.criticalEye + desc.exploitWeakness);

	if (criticalProbability.x > 1) { criticalProbability.x = 1; }
	if (criticalProbability.x < -1) { criticalProbability.x = -1; }

	if (criticalProbability.y > 1) { criticalProbability.y = 1; }
	if (criticalProbability.y < -1) { criticalProbability.y = -1; }

	auto absCriticalProb = Core::Vector2D(
		abs(criticalProbability.x), 
		abs(criticalProbability.y));

	// 물리 회심 배율
	auto physicalCriticalRate = Core::Vector2D(
		BaseCriticalDamageRate(criticalProbability.x) + desc.superCritical,
		BaseCriticalDamageRate(criticalProbability.y) + desc.superCritical);

	// 속성 회심 배율
	auto elementalCriticalRate = 1 + desc.fireDragonGambit;

	// 비회심 기대 대미지
	Core::Vector2D expectedDamage = modifiedBaseDamage;

	// 회심 기대 대미지
	Core::Vector2D criticalExpectedDamageNormal(
		modifiedBaseDamage.x * physicalCriticalRate.x,
		modifiedBaseDamage.y * elementalCriticalRate);

	Core::Vector2D criticalExpectedDamageWeakness(
		modifiedBaseDamage.x * physicalCriticalRate.y,
		modifiedBaseDamage.y * elementalCriticalRate);

	// 모션치 적용
	Core::Vector2D motionDamage =
		expectedDamage *
		Core::Vector2D(weapon.MotionValue(desc), 1);
	Core::Vector2D criticalMotionDamageNormal =
		criticalExpectedDamageNormal *
		Core::Vector2D(weapon.MotionValue(desc), 1);
	Core::Vector2D criticalMotionDamageWeakness =
		criticalExpectedDamageWeakness *
		Core::Vector2D(weapon.MotionValue(desc), 1);

	// 방어력 적용
	Core::Vector2D appliedDamageNormal = motionDamage * monster.normalDefense;
	Core::Vector2D criticalAppliedDamageNormal = criticalMotionDamageNormal * monster.normalDefense;

	Core::Vector2D appliedDamageWeakness = motionDamage * monster.weakDefense;
	Core::Vector2D criticalAppliedDamageWeakness = criticalMotionDamageWeakness * monster.weakDefense;

	// 최종 대미지 (비약점/약점)
	Core::Vector2D expectedDamageNormal =
		Core::Vector2D(
			int(appliedDamageNormal.x) * (1 - absCriticalProb.x) +
			int(criticalAppliedDamageNormal.x) * absCriticalProb.x,
			int(appliedDamageNormal.y) * (1 - absCriticalProb.x) +
			int(criticalAppliedDamageNormal.y) * absCriticalProb.x);

	Core::Vector2D expectedDamageWeakness =
		Core::Vector2D(
			int(appliedDamageWeakness.x) * (1 - absCriticalProb.y) +
			int(criticalAppliedDamageWeakness.x) * absCriticalProb.y,
			int(appliedDamageWeakness.y) * (1 - absCriticalProb.y) +
			int(criticalAppliedDamageWeakness.y) * absCriticalProb.y);

	// 기대 대미지
	Core::Vector2D finalExpectedDamage =
		Core::Vector2D(
			monster.weakHitPercent * expectedDamageWeakness.x +
			(1 - monster.weakHitPercent) * expectedDamageNormal.x,
			monster.weakHitPercent * expectedDamageWeakness.y +
			(1 - monster.weakHitPercent) * expectedDamageNormal.y);

	if (file != nullptr)
	{
		fwprintf(
			file,
			L"%.3f\t%.3f\t"
			"%.3f\t%.3f\t"

			"%.3f\t%.3f\t"
			"%.3f\t%.3f\t"
			"%.3f\t"

			"%.3f\t%.3f\t"
			"%.3f\t%.3f\t"
			"%.3f\t%.3f\t"

			"%.3f\t%.3f\t"
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

			criticalProbability.x, criticalProbability.y,
			physicalCriticalRate.x, physicalCriticalRate.y,
			elementalCriticalRate,

			expectedDamage.x, expectedDamage.y,
			criticalExpectedDamageNormal.x, criticalExpectedDamageNormal.y,
			criticalExpectedDamageWeakness.x, criticalExpectedDamageWeakness.y,

			motionDamage.x, motionDamage.y,
			criticalMotionDamageNormal.x, criticalMotionDamageNormal.y,
			criticalMotionDamageWeakness.x, criticalMotionDamageWeakness.y,

			appliedDamageNormal.x, appliedDamageNormal.y,
			criticalAppliedDamageNormal.x, criticalAppliedDamageNormal.y,

			appliedDamageWeakness.x, appliedDamageWeakness.y,
			criticalAppliedDamageWeakness.x, criticalAppliedDamageWeakness.y,

			finalExpectedDamage.x, finalExpectedDamage.y,
			finalExpectedDamage.x + finalExpectedDamage.y);
	}

	return finalExpectedDamage.x + finalExpectedDamage.y;
}
