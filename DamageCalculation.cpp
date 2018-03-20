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

		if (name == L"����") { attackBonus = AttackSkillBonus(lv); }
		else if (name == L"����") { criticalEye = CriticalEye(lv); }
		else if (name == L"���� ȸ��") { superCritical = SuperCritical(lv); }
		else if (name == L"���� Ưȿ") { exploitWeakness = ExploitWeakness(lv); }

		else if (name == L"�ҼӼ� ���� ��ȭ") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"���Ӽ� ���� ��ȭ") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"�����Ӽ� ���� ��ȭ") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"�����Ӽ� ���� ��ȭ") { elementalBonus = ElementalSkillLevel(lv); }
		else if (name == L"��Ӽ� ���� ��ȭ") { elementalBonus = ElementalSkillLevel(lv); }

		else if (name == L"���Ӽ� ��ȭ") { nonElementalBonus = NonElementalBonus(lv); }
		else if (name == L"ȭ���� ���") { fireDragonGambit = lv >= 2 ? FireDragonsGambit() : 0; }
		else if (name == L"Ȱ ������ �ܰ� ����")  { chargeLevel = lv >= 1 ? 3 : 2;  }
		else if (name == L"���ź/���ȭ�� ��ȭ") { normal = lv >= 1; }
		else if (name == L"��ź/���� ��ȭ") { spread = lv >= 1; }
		else if (name == L"Ư�� ��� ��ȭ") { specialAttack = SpecialAttackBonus(lv); }
		else if (name == L"����ź/��ȭ�� ��ȭ") { penetration = lv >= 1 ? 0.1 : 0; }
		else if (name == L"�ߵ��� [��]") { drawCritical = DrawCriticalBonus(lv); }
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
	// ���� �⺻ �����
	Core::Vector2D rawDamage = weapon.baseDamage;
	rawDamage.x /= weapon.multiplier;
	rawDamage.y /= 10;

	// ����� ���ʽ�
	Core::Vector2D rawDamageWithBonus =
		rawDamage +
		Core::Vector2D(
			desc.attackBonus.first,
			desc.elementalBonus.first / 10);

	// ���ݺ� ������ ���� �����,
	// ������ 1.5��� �˷��� �ִµ� �Ʒ��� ��ġ�� �����غ��� 1.35��
	double strongBottle = weapon.strongBottle ? 1.35 : 1.0;

	// �⺻ ����� * ȭ�� ��ȭ * ���ݺ� * �Ÿ� ũ��Ƽ��
	double arrowUpgrade =
		(1 + desc.arrowUpgrade) *		// ���ź/��ź ��ȭ
		(1 + desc.specialAttack) *		// Ư�� ��� ��ȭ
		(1 + desc.penetration) *		// ���� ��ȭ
		(1 + desc.nonElementalBonus);	// ���Ӽ� ��ȭ

	Core::Vector2D modifiedBaseDamage(
		rawDamageWithBonus.x * arrowUpgrade * strongBottle * 1.5,
		rawDamageWithBonus.y * (1 + desc.elementalBonus.second));

	if (modifiedBaseDamage.y >= rawDamage.y * 1.3)
	{
		modifiedBaseDamage.y = rawDamage.y * 1.3;
	}

	// ȸ�ɷ� (�����, ����)
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

	// ���� ȸ�� ����
	auto physicalCriticalRate = Core::Vector2D(
		BaseCriticalDamageRate(criticalProbability.x) + desc.superCritical,
		BaseCriticalDamageRate(criticalProbability.y) + desc.superCritical);

	// �Ӽ� ȸ�� ����
	auto elementalCriticalRate = 1 + desc.fireDragonGambit;

	// ��ȸ�� ��� �����
	Core::Vector2D expectedDamage = modifiedBaseDamage;

	// ȸ�� ��� �����
	Core::Vector2D criticalExpectedDamageNormal(
		modifiedBaseDamage.x * physicalCriticalRate.x,
		modifiedBaseDamage.y * elementalCriticalRate);

	Core::Vector2D criticalExpectedDamageWeakness(
		modifiedBaseDamage.x * physicalCriticalRate.y,
		modifiedBaseDamage.y * elementalCriticalRate);

	// ���ġ ����
	Core::Vector2D motionDamage =
		expectedDamage *
		Core::Vector2D(weapon.MotionValue(desc), 1);
	Core::Vector2D criticalMotionDamageNormal =
		criticalExpectedDamageNormal *
		Core::Vector2D(weapon.MotionValue(desc), 1);
	Core::Vector2D criticalMotionDamageWeakness =
		criticalExpectedDamageWeakness *
		Core::Vector2D(weapon.MotionValue(desc), 1);

	// ���� ����
	Core::Vector2D appliedDamageNormal = motionDamage * monster.normalDefense;
	Core::Vector2D criticalAppliedDamageNormal = criticalMotionDamageNormal * monster.normalDefense;

	Core::Vector2D appliedDamageWeakness = motionDamage * monster.weakDefense;
	Core::Vector2D criticalAppliedDamageWeakness = criticalMotionDamageWeakness * monster.weakDefense;

	// ���� ����� (�����/����)
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

	// ��� �����
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
