#include "pch.h"
#include "DamageCalculation.h"

#include "Vector.h"

#include "Skill.h"
#include "CombinationBase.h"

//------------------------------------------------------------------------------
Desc::Desc(
	const EvaluatingSkills& evSkills, 
	const CombinationBase* comb)
{
	bool normal = false;
	bool spread = false;

	for (int i = 0; i < comb->skillCount; ++i)
	{
		const auto& name = evSkills.list[i].name;
		int lv = comb->skills[i];

		if (name == L"공격") { attackBonus = AttackSkillBonus(lv); }
		else if (name == L"간파") { criticalEye = CriticalEye(lv); }
		else if (name == L"슈퍼 회심") { superCritical = SuperCritical(lv); }
		else if (name == L"약점 특효") { exploitWeakness = ExploitWeakness(lv); }

		else if (name == L"불속성 공격 강화") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"물속성 공격 강화") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"번개속성 공격 강화") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"얼음속성 공격 강화") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"용속성 공격 강화") { elementalBonus = ElementalSkillLevel(lv); }

		else if (name == L"무속성 강화") { nonElementalBonus = NonElementalBonus(lv); }
		else if (name == L"화룡의 비기") { fireDragonGambit = lv >= 2 ? FireDragonsGambit() : 0; }
		else if (name == L"활 모으기 단계 해제")  { chargeLevel = lv >= 1 ? 3 : 2;  }
		else if (name == L"통상탄/통상화살 강화") { normal = lv >= 1; }
		else if (name == L"산탄/강사 강화") { spread = lv >= 1; }
		else if (name == L"특수 사격 강화") { specialAttack = SpecialAttackBonus(lv); }
		else if (name == L"관통탄/용화살 강화") { penetration = lv >= 1 ? 0.1 : 0; }
		else if (name == L"발도술 [기]") { drawCritical = DrawCriticalBonus(lv); }
	}

	if (normal && spread) { this->arrowUpgrade = 0.1; }
}

//------------------------------------------------------------------------------
inline double BaseCriticalDamageRate(double criticalRate) 
{ 
	return criticalRate >= -0.01 ? 1.25 : 0.75; 
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

	// 강격병 유무에 따른 대미지,
	// 배율이 1.5배로 알려져 있는데 훈련장 수치를 역산해보니 1.35임
	double strongBottle = weapon.strongBottle ? 1.35 : 1.0;

	// 기본 대미지 * 화살 강화 * 강격병 * 거리 크리티컬
	double arrowUpgrade =
		(1 + desc.arrowUpgrade) *		// 통상탄/산탄 강화
		(1 + desc.specialAttack) *		// 특수 사격 강화
		(1 + desc.penetration) *		// 관통 강화
		(1 + desc.nonElementalBonus);	// 무속성 강화

	Core::Vector2D modifiedBaseDamage(
		rawDamageWithBonus.x * arrowUpgrade * strongBottle * 1.5,
		rawDamageWithBonus.y * (1 + desc.elementalBonus.second));

	if (modifiedBaseDamage.y >= rawDamage.y * 1.3)
	{
		modifiedBaseDamage.y = rawDamage.y * 1.3;
	}

	// 회심률 (비약점, 약점)
	auto criticalProbability = Core::Vector2D(
		weapon.criticalRate + desc.attackBonus.second + desc.criticalEye + desc.drawCritical, 
		weapon.criticalRate + desc.attackBonus.second + desc.criticalEye + desc.drawCritical + desc.exploitWeakness);

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
