#include "pch.h"
#include "CombinationBase.h"

#include <WindowsUtility.h>
#include <Utility.h>

#include "Skill.h"
#include "Decorator.h"

using namespace std;

//------------------------------------------------------------------------------
CombinationBase::CombinationBase(const CombinationBase& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
CombinationBase::CombinationBase(int skillCount)
	: skillCount(skillCount)
{
	skills = new int[skillCount] { 0 };

	memset(slots, 0, sizeof(int) * COUNT_OF(slots));
}

//------------------------------------------------------------------------------
CombinationBase::~CombinationBase()
{
	delete[] skills;
}

//------------------------------------------------------------------------------
CombinationBase& CombinationBase::operator = (const CombinationBase& rhs)
{
	delete[] skills;

	skillCount = rhs.skillCount;
	skills = new int[skillCount];

	memcpy(skills, rhs.skills, sizeof(int) * skillCount);

	memcpy(slots, rhs.slots, sizeof(int) * COUNT_OF(slots));

	return *this;
}

//------------------------------------------------------------------------------
int CombinationBase::SlotCount() const
{
	return slots[0] + slots[1] + slots[2];
}

//------------------------------------------------------------------------------
static void CalculateFreedom(
	const CombinationBase** side,
	int slotSize,
	int lhsSocketToUse, 
	int* freedom,
	bool* maxed)
{
	for (int s = 0; s < 2; ++s)
	{
		int slotAvailable = side[s]->slots[slotSize];

		if (s == 0 && lhsSocketToUse == slotSize)
		{
			slotAvailable--;
		}

		for (auto si : g_skillsBySlotSize[slotSize])
		{
			int lvDiff = g_skillMaxLevel[si] - side[s]->skills[si];

			int toUse = min(lvDiff, slotAvailable);

			slotAvailable -= toUse;
			freedom[s] += toUse;
			if (slotAvailable <= 0) { break; }
		}

		if (slotAvailable > 0)
		{
			maxed[s] = true;
		}
	}
}

//------------------------------------------------------------------------------
int GetGeneralSize(int* freedom, bool* maxed)
{
	if (maxed[0])
	{
		if (maxed[1])
		{
			// ���ʿ� ���� ũ�� ���Կ� ����ָ� �� ä���
			// �ʿ��� ��ų ������ ��� �ִ�� ���� �� �ִ�.
			// �̷��� ���� ����� �ʿ� ���� ������ ������ �ɷ� �����Ѵ�
			return - 2;
		}
		else
		{
			// �̷��� ������ ������ ���� �� ������ ������ �����̴�
			return 0;
		}
	}
	else
	{
		if (maxed[1])
		{
			// �̷��� ������ ������ ���� �� ������ ������ �����̴�
			return 1;
		}
		else
		{
			if (freedom[0] > freedom[1])
			{
				return 0;
			}
			else if (freedom[0] < freedom[1])
			{
				return 1;
			}
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
	CombinationBase::CompareStrict2(
		const CombinationBase& lhs,
		const CombinationBase& rhs)
{
	return CompareStrict2(lhs, rhs, -1, -1);
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
	CombinationBase::CompareStrict2(
		const CombinationBase& lhs,
		const CombinationBase& rhs,
		int skillToAddToLhs,
		int lhsSocketToUse)
{
	const CombinationBase* side[] = { &lhs, &rhs };

	bool canExpressOther[] = { true, true };

#	undef CHECK_SHORTCIRCUIT
#	define CHECK_SHORTCIRCUIT() \
		{ if (!canExpressOther[0] && !canExpressOther[1]) { return CombinationBase::Undetermined; } }

	// Strict = �� ���� �ڸ����� �ش� ũ���� ����ָ� ����ٰ� ����

	// ���� ����ַ� �ذ���� �ʴ� ��ų�� ���Ѵ�
	for (auto si : g_skillWithNoDecorator)
	{
		if (lhs.skills[si] > rhs.skills[si])
		{
			canExpressOther[1] = false;
			CHECK_SHORTCIRCUIT();
		}
		else if (rhs.skills[si] > lhs.skills[si])
		{
			canExpressOther[0] = false;
			CHECK_SHORTCIRCUIT();
		}
	}

	// ��ų ���¸� �����Ѵ�
	const int bufferSize = 32;
	int skills[2][bufferSize];
	assert(COUNT_OF(g_skills) < bufferSize);

	if (skillToAddToLhs >= 0) { skills[0][skillToAddToLhs]++; }

	for (int i = 0; i < COUNT_OF(side); ++i)
	{
		memcpy(skills[i], side[i]->skills, sizeof(int) * COUNT_OF(g_skills));
	}

	// ���Ժ� �������� �������� ��ȣ ǥ�� ���ɼ��� �����Ѵ�
	for (int slotSize = 0; slotSize < 3; ++slotSize)
	{
		// ���� ���� ũ�⺰ �������� ����Ѵ�
		int freedom[] = { 0, 0, };
		bool maxed[] = { false, false, };
		CalculateFreedom(side, slotSize, lhsSocketToUse, freedom, maxed);

		// ������ �������� ���� ���¿��� �񱳿� ����
		// ������ ��� ��ų �ִ��� ��� ũ�� �񱳰� ���ǹ��ϴ�
		auto moreGenericSide = GetGeneralSize(freedom, maxed);

		if (moreGenericSide == -2)
		{
			// ���� ��� ����ָ� �ȾƼ� �ʿ��� ��ų ���� ��� ������ ��Ȳ
			continue;
		}
		else
		{
			int available[] = { freedom[0], freedom[1], };
			int totalLevel[] = { freedom[0], freedom[1], };

			for (auto si : g_skillsBySlotSize[slotSize])
			{
				totalLevel[0] += side[0]->skills[si];
				totalLevel[1] += side[1]->skills[si];
			}

			// ��� ��ų ������ ���� ���� ���� �ٸ� ���� ǥ���� �� ����
			if (totalLevel[0] > totalLevel[1])
			{
				canExpressOther[1] = false;
				CHECK_SHORTCIRCUIT();
			}
			else if (totalLevel[0] < totalLevel[1])
			{
				canExpressOther[0] = false;
				CHECK_SHORTCIRCUIT();
			}

			// ���ʿ� ����ָ� �ȾƼ� �ٸ� ���� ���� �� �ִ��� Ȯ���Ѵ�
			for (int s = 0; s < 2; ++s)
			{
				int o = 1 - s;

				for (auto si : g_skillsBySlotSize[slotSize])
				{
					if (side[s]->skills[si] < side[o]->skills[si])
					{
						auto delta = side[o]->skills[si] - side[s]->skills[si];
						if (delta > available[s])
						{
							canExpressOther[s] = false;
							CHECK_SHORTCIRCUIT();
							break;
						}
						else
						{
							available[s] -= delta;
						}
					}
				}

				// ����
				available[s] = freedom[s];
			}

			// ������ Undetermined�� �Ⱒ���� �ʾҴٸ� 
			// �� ������ ������ ��� ������� �� 
			// �� ��ų�� �޼� ���� �ִ� ������ ������ ���Ѵ�
			// ��� ������ �ٸ� �ʺ��� �׻� ������ ����/�Ҹ��� ���� �� �ִ�
			char checked[bufferSize] = { 0 };
			assert(g_skillsBySlotSize[slotSize].size() < bufferSize);

			function<bool()> EvaluatePermutations;

			EvaluatePermutations = [&]() -> bool
			{
				for (auto si : g_skillsBySlotSize[slotSize])
				{
					if (checked[si] != 0) { continue; }

					if (skills[0][si] >= g_skillMaxLevel[si] &&
						skills[1][si] >= g_skillMaxLevel[si])
					{
						continue;
					}

					int toUse[] =
					{
						min(g_skillMaxLevel[si] - skills[0][si], available[0]),
						min(g_skillMaxLevel[si] - skills[1][si], available[1]),
					};

					skills[0][si] += toUse[0]; available[0] -= toUse[0];
					skills[1][si] += toUse[1]; available[1] -= toUse[1];

					// ����ָ� �ȾƼ� �޼� ������ �ִ� ������ 
					// ������ �� ���� �̿��ؼ� �ٸ� �� ���� ������ �� ����
					if (skills[0][si] > skills[1][si])
					{
						canExpressOther[1] = false;
						CHECK_SHORTCIRCUIT();
					}
					else if (skills[0][si] < skills[1][si])
					{
						canExpressOther[0] = false;
						CHECK_SHORTCIRCUIT();
					}
					else
					{
						// ���� �� ���� �� �� �ִ�
					}

					if (available[0] > 0 || available[1] > 0)
					{
						checked[si] = 1;

						if (EvaluatePermutations())
						{
							return true;
						}

						checked[si] = 0;
					}
					else
					{
						if (available[0] > available[1])
						{
							canExpressOther[1] = false;
							CHECK_SHORTCIRCUIT();
						}
						else if (available[0] < available[1])
						{
							canExpressOther[0] = false;
							CHECK_SHORTCIRCUIT();
						}
					}

					skills[0][si] -= toUse[0]; available[0] += toUse[0];
					skills[1][si] -= toUse[1]; available[1] += toUse[1];
				}

				return false;
			};

			if (EvaluatePermutations())
			{
				CHECK_SHORTCIRCUIT();
			}
		}
	}

#	undef CHECK_SHORTCIRCUIT

	if (canExpressOther[0])
	{
		return canExpressOther[1] ? 
			CombinationBase::Equal : CombinationBase::Better;
	}
	else
	{
		return canExpressOther[1] ?
			CombinationBase::Worse : CombinationBase::Undetermined;
	}
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
CombinationBase::Compare(
	const CombinationBase& lhs,
	const CombinationBase& rhs)
{
	return Compare(lhs, rhs, -1, -1);
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
CombinationBase::Compare(
	const CombinationBase& lhs,
	const CombinationBase& rhs,
	int lhsSkillToAdd,
	int lhsSocketToUse)
{
	assert(lhs.skillCount == rhs.skillCount);
	int skillCount = lhs.skillCount;

	// ���� ���� ���� üũ
	bool exactSame = true;

	// ����� ���ؼ� �����Ѵ�
	int slotLhs[3] = { lhs.slots[0], lhs.slots[1], lhs.slots[2], };
	int slotRhs[3] = { rhs.slots[0], rhs.slots[1], rhs.slots[2], };

	if (lhsSocketToUse >= 0 && lhsSocketToUse < COUNT_OF(slotLhs))
	{
		slotLhs[lhsSocketToUse]--;
	}

	bool notWorseThan[2] = { false, false };

	// ��ų�� ������ ���� ���Ѵ�
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int lhsSkill = lhs.skills[i] + (i == lhsSkillToAdd ? 1 : 0);
		int rhsSkill = rhs.skills[i];

		// ���� ���� ���θ� Ȯ���Ѵ�
		exactSame = exactSame && (lhsSkill == rhsSkill);

		if (lhsSkill == 0 && rhsSkill == 0) { continue; }

		int toSub = min(lhsSkill, rhsSkill);
		lhsSkill = lhsSkill - toSub;
		rhsSkill = rhsSkill - toSub;

		if (lhsSkill == 0 && rhsSkill == 0) { continue; }

		if (lhsSkill > 0) { assert(rhsSkill == 0); }
		else { assert(lhsSkill == 0); }

		int skillLeft = lhsSkill > 0 ? lhsSkill : rhsSkill;
		int* slotsInOtherSide = lhsSkill > 0 ? slotRhs : slotLhs;
		int index = lhsSkill > 0 ? 0 : 1;

		// ��� �� �ʿ� ���� ��ų�� 
		// �ݴ��ʿ� ����ָ� �޾Ƽ� Ŀ�� �������� Ȯ���Ѵ�
		if (g_skillToDecorator[i] != nullptr)
		{
			int slotSize = g_skillToDecorator[i]->slotSize;

			for (int j = slotSize - 1; j < COUNT_OF(slots); ++j)
			{
				int toSub = min(skillLeft, slotsInOtherSide[j]);

				skillLeft -= toSub;
				slotsInOtherSide[j] -= toSub;
				if (skillLeft <= 0) { break; }
			}

			if (skillLeft > 0)
			{
				notWorseThan[index] = true;
				if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
			}
		}
		else
		{
			// ����ַ� Ŀ���� �� �Ǵ� ��ų�̴�
			notWorseThan[index] = true;
			if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
		}
	}

	// ��ȣ ������ ���Ѵ�
	if (lhs.SlotCount() > 0 || rhs.SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			// ���� ���� ���θ� Ȯ���Ѵ�
			exactSame = exactSame && (lhs.slots[slotSize] == rhs.slots[slotSize]);

			for (int j = 0; j < 2; ++j)
			{
				int curSlot = j == 0 ?
					lhs.slots[slotSize] : rhs.slots[slotSize];

				if (j == 0 && slotSize == lhsSocketToUse)
				{
					curSlot--;
				}

				if (curSlot == 0) { continue; }

				int* otherSide = j == 0 ? slotRhs : slotLhs;

				bool selected = false;
				for (int i = slotSize; i < COUNT_OF(slots); ++i)
				{
					int toSub = min(curSlot, otherSide[i]);

					curSlot -= toSub;
					otherSide[i] -= toSub;

					if (curSlot <= 0)
					{
						break;
					}
				}

				if (curSlot > 0)
				{
					notWorseThan[j] = true;
					if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
				}
			}
		}
	}

	if (exactSame)
	{
		assert(!notWorseThan[0] && !notWorseThan[1]);
	}

	if (notWorseThan[0])
	{
		return notWorseThan[1] ? Undetermined : Better;
	}
	else
	{
		return notWorseThan[1] ? Worse : Equal;
	}
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
CombinationBase::CompareStrict(
	const CombinationBase& lhs,
	const CombinationBase& rhs)
{
	return Compare(lhs, rhs, -1, -1);
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
CombinationBase::CompareStrict(
	const CombinationBase& lhs,
	const CombinationBase& rhs,
	int lhsSkillToAdd,
	int lhsSocketToUse)
{
	assert(lhs.skillCount == rhs.skillCount);
	int skillCount = lhs.skillCount;

	// ���� ���� ���� üũ
	bool exactSame = true;

	// ����� ���ؼ� �����Ѵ�
	bool notWorseThan[2] = { false, false };

	// ��ų�� ������ ���� ���Ѵ�
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int lhsSkill = lhs.skills[i] + (i == lhsSkillToAdd ? 1 : 0);
		int rhsSkill = rhs.skills[i];

		// ���� ���� ���θ� Ȯ���Ѵ�
		exactSame = exactSame && (lhsSkill == rhsSkill);

		if (lhsSkill < rhsSkill)
		{
			notWorseThan[1] = true;
			if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
		}
		else if (lhsSkill > rhsSkill)
		{
			notWorseThan[0] = true;
			if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
		}
	}

	// ��ȣ ������ ���Ѵ�
	if (lhs.SlotCount() > 0 || rhs.SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			int lhsSlot = lhs.slots[slotSize] - (slotSize == lhsSocketToUse ? 1 : 0);
			int rhsSlot = rhs.slots[slotSize];

			// ���� ���� ���θ� Ȯ���Ѵ�
			exactSame = exactSame && (lhsSlot == rhsSlot);

			if (lhsSlot < rhsSlot)
			{
				notWorseThan[1] = true;
				if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
			}
			else if (lhsSlot > rhsSlot)
			{
				notWorseThan[0] = true;
				if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
			}
		}
	}

#if 0
	assert(lhs.skillCount == rhs.skillCount);
	int skillCount = lhs.skillCount;

	// ���� ���� ���� üũ
	bool exactSame = true;

	// ����� ���ؼ� �����Ѵ�
	int slotLhs[3] = { lhs.slots[0], lhs.slots[1], lhs.slots[2], };
	int slotRhs[3] = { rhs.slots[0], rhs.slots[1], rhs.slots[2], };

	bool notWorseThan[2] = { false, false };

	// ��ų�� ������ ���� ���Ѵ�
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int lhsSkill = lhs.skills[i] + (i == lhsSkillToAdd ? 1 : 0);
		int rhsSkill = rhs.skills[i];

		// ���� ���� ���θ� Ȯ���Ѵ�
		exactSame = exactSame && (lhsSkill == rhsSkill);

		if (lhsSkill == 0 && rhsSkill == 0) { continue; }

		int toSub = min(lhsSkill, rhsSkill);
		lhsSkill = lhsSkill - toSub;
		rhsSkill = rhsSkill - toSub;

		if (lhsSkill == 0 && rhsSkill == 0) { continue; }

		if (lhsSkill > 0) { assert(rhsSkill == 0); }
		else { assert(lhsSkill == 0); }

		int skillLeft = lhsSkill > 0 ? lhsSkill : rhsSkill;
		int* slotsInOtherSide = lhsSkill > 0 ? slotRhs : slotLhs;
		int index = lhsSkill > 0 ? 0 : 1;

		// ��� �� �ʿ� ���� ��ų�� 
		// �ݴ��ʿ� ����ָ� �޾Ƽ� Ŀ�� �������� Ȯ���Ѵ�
		if (g_skillToDecorator[i] != nullptr)
		{
			int slotSize = g_skillToDecorator[i]->slotSize;

			// ��Ȯ�� �� ���Կ� �¾ƾ� �Ѵ�
			int toSub = min(skillLeft, slotsInOtherSide[slotSize - 1]);

			skillLeft -= toSub;
			slotsInOtherSide[slotSize - 1] -= toSub;

			if (skillLeft > 0)
			{
				notWorseThan[index] = true;
				if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
			}
		}
		else
		{
			// ����ַ� Ŀ���� �� �Ǵ� ��ų�̴�
			notWorseThan[index] = true;
			if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
		}
	}

	// ��ȣ ������ ���Ѵ�
	if (lhs.SlotCount() > 0 || rhs.SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			// ���� ���� ���θ� Ȯ���Ѵ�
			exactSame = exactSame && (lhs.slots[slotSize] == rhs.slots[slotSize]);

			for (int j = 0; j < 2; ++j)
			{
				int curSlot = j == 0 ? lhs.slots[slotSize] : rhs.slots[slotSize];

				if (j == 0 && slotSize == lhsSocketToUse)
				{
					curSlot--;
				}

				if (curSlot == 0) { continue; }

				int* otherSide = j == 0 ? slotRhs : slotLhs;

				bool selected = false;
				int toSub = min(curSlot, otherSide[slotSize]);

				curSlot -= toSub;
				otherSide[slotSize] -= toSub;

				if (curSlot > 0)
				{
					notWorseThan[j] = true;
					if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
				}
			}
		}
	}
#endif

	if (exactSame)
	{
		assert(!notWorseThan[0] && !notWorseThan[1]);
	}

	if (notWorseThan[0])
	{
		return notWorseThan[1] ? Undetermined : Better;
	}
	else
	{
		return notWorseThan[1] ? Worse : Equal;
	}
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
CombinationBase::Compare(
	const CombinationBase& rhs) const
{
	// ���� ���� ���� üũ
	bool exactSame = true;

	// ����� ���ؼ� �����Ѵ�
	int slotRhs[3] = { rhs.slots[0], rhs.slots[1],rhs.slots[2], };

	// ��ų�� ������ ���� ���Ѵ�
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int curSkill = skills[i];

		// ���� ���� ���θ� Ȯ���Ѵ�
		exactSame = exactSame && (curSkill == rhs.skills[i]);

		if (curSkill > 0)
		{
			int toSub = min(curSkill, rhs.skills[i]);
			curSkill = curSkill - toSub;

			// lhs�� ���� ��ų�� rhs�� ����ָ� �޾Ƽ� Ŀ�� �������� Ȯ���Ѵ�
			if (curSkill > 0)
			{
				if (g_skillToDecorator[i] != nullptr)
				{
					int slotSize = g_skillToDecorator[i]->slotSize;

					for (int j = slotSize - 1; j < COUNT_OF(slots); ++j)
					{
						int toSub = min(curSkill, slotRhs[j]);

						if (toSub > 0)
						{
							curSkill -= toSub;
							slotRhs[j] -= toSub;

							if (curSkill <= 0)
							{
								break;
							}
						}
					}

					if (curSkill > 0)
					{
						return NotWorse;
					}
				}
				else
				{
					// ����ַ� Ŀ���� �� �Ǵ� ��ų�̴�
					// �� ������ ��¿ �� ���� ����� �Ѵ�
					return NotWorse;
				}
			}
		}
	}

	// lhs�� ���Կ� ���� �� �ִ� ����ָ�
	// rhs�� ���Կ��� ���� �� �ִ��� Ȯ���Ѵ�
	if (SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			int curSlot = slots[slotSize];

			// ���� ���� ���θ� Ȯ���Ѵ�
			exactSame = exactSame && (curSlot == rhs.slots[slotSize]);

			if (curSlot > 0)
			{
				bool selected = false;
				for (int i = slotSize; i < COUNT_OF(slots); ++i)
				{
					int toSub = min(curSlot, slotRhs[i]);

					if (toSub > 0)
					{
						curSlot -= toSub;
						slotRhs[i] -= toSub;

						if (curSlot <= 0)
						{
							break;
						}
					}
				}

				if (curSlot > 0)
				{
					return NotWorse;
				}
			}
		}
	}
	else
	{
		// ���� ���� ���θ� Ȯ���Ѵ�
		exactSame = exactSame && (rhs.SlotCount() == 0);
	}

	return exactSame ? Equal : Worse;
}

//------------------------------------------------------------------------------
bool CombinationBase::IsWorseThanOrEqualTo(const CombinationBase& rhs) const
{
	auto result = Compare(rhs);
	return result != NotWorse;
}

//------------------------------------------------------------------------------
void CombinationBase::Combine(const CombinationBase& rhs)
{
	for (int i = 0; i < skillCount; ++i)
	{
		skills[i] += rhs.skills[i];

		if (skills[i] > g_skillMaxLevel[i])
		{
			skills[i] = g_skillMaxLevel[i];
		}
	}

	for (int i = 0; i < COUNT_OF(slots); ++i)
	{
		slots[i] += rhs.slots[i];
	}
}

//------------------------------------------------------------------------------
bool CombinationBase::IsTriviallyWorse(const CombinationBase& rhs) const
{
	for (int i = 0; i < skillCount; ++i)
	{
		if (skills[i] > rhs.skills[i]) { return false; }
	}

	for (int i = 0; i < COUNT_OF(slots); ++i)
	{
		if (slots[i] > rhs.slots[i]) { return false; }
	}

	return true;
}

//------------------------------------------------------------------------------
bool CombinationBase::IsTriviallyEquivalent(const CombinationBase& rhs) const
{
	for (int i = 0; i < skillCount; ++i)
	{
		if (skills[i] != rhs.skills[i]) { return false; }
	}

	for (int i = 0; i < COUNT_OF(slots); ++i)
	{
		if (slots[i] != rhs.slots[i]) { return false; }
	}

	return true;
}

//------------------------------------------------------------------------------
void CombinationBase::Dump() const
{
	DumpSimple();

	WindowsUtility::Debug(L"\n");
}

//------------------------------------------------------------------------------
void CombinationBase::Dump(FILE* file) const
{
	fwprintf(file, L"%s\n", DumpToString().c_str());
}

//------------------------------------------------------------------------------
wstring CombinationBase::DumpToString() const
{
	wstring result;

	bool first = true;
	for (int i = 0; i < skillCount; ++i)
	{
		if (skills[i] > 0)
		{
			result += Utility::FormatW(
				first ? L"%s%d" : L" %s%d",
				g_skillsAbb[i].c_str(),
				skills[i]);
			first = false;
		}
	}

	for (int i = 0; i < COUNT_OF(slots); ++i)
	{
		//for (int j = 0; j < slots[i]; ++j)
		//{
		//	WindowsUtility::Debug(L" [%d]", i + 1);
		//}
		result += Utility::FormatW(L" (%d)", slots[i]);
	}

	return result;
}

//------------------------------------------------------------------------------
void CombinationBase::DumpSimple() const
{
	bool first = true;
	for (int i = 0; i < skillCount; ++i)
	{
		if (skills[i] > 0)
		{
			WindowsUtility::Debug(
				first ? L"%s%d" : L" %s%d",
				g_skillsAbb[i].c_str(),
				skills[i]);
			first = false;
		}
	}

	for (int i = 0; i < COUNT_OF(slots); ++i)
	{
		//for (int j = 0; j < slots[i]; ++j)
		//{
		//	WindowsUtility::Debug(L" [%d]", i + 1);
		//}
		WindowsUtility::Debug(L" (%d)", slots[i]);
	}
}
