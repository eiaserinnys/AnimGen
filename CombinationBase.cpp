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
			// 양쪽에 같은 크기 슬롯에 장식주를 다 채우면
			// 필요한 스킬 레벨을 모두 최대로 만들 수 있다.
			// 이러면 서로 고민할 필요 없이 완전히 동등한 걸로 간주한다
			return - 2;
		}
		else
		{
			// 이러면 좌측은 만렙을 만들 수 있으니 유리한 조합이다
			return 0;
		}
	}
	else
	{
		if (maxed[1])
		{
			// 이러면 우측은 만렙을 만들 수 있으니 유리한 조합이다
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

	// Strict = 각 슬롯 자리에는 해당 크기의 장식주만 끼운다고 가정

	// 먼저 장식주로 해결되지 않는 스킬을 비교한다
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

	// 스킬 상태를 복제한다
	const int bufferSize = 32;
	int skills[2][bufferSize];
	assert(COUNT_OF(g_skills) < bufferSize);

	if (skillToAddToLhs >= 0) { skills[0][skillToAddToLhs]++; }

	for (int i = 0; i < COUNT_OF(side); ++i)
	{
		memcpy(skills[i], side[i]->skills, sizeof(int) * COUNT_OF(g_skills));
	}

	// 슬롯별 자유도를 바탕으로 상호 표현 가능성을 검증한다
	for (int slotSize = 0; slotSize < 3; ++slotSize)
	{
		// 먼저 슬롯 크기별 자유도를 계산한다
		int freedom[] = { 0, 0, };
		bool maxed[] = { false, false, };
		CalculateFreedom(side, slotSize, lhsSocketToUse, freedom, maxed);

		// 양측의 자유도를 구한 상태에서 비교에 들어간다
		// 가용한 모든 스킬 최대인 경우 크기 비교가 무의미하다
		auto moreGenericSide = GetGeneralSize(freedom, maxed);

		if (moreGenericSide == -2)
		{
			// 양쪽 모두 장식주를 꽂아서 필요한 스킬 만렙 찍는 해피한 상황
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

			// 모든 스킬 레벨의 합이 적은 쪽은 다른 쪽을 표현할 수 없다
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

			// 한쪽에 장식주를 꽂아서 다른 쪽을 만들 수 있는지 확인한다
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

				// 원복
				available[s] = freedom[s];
			}

			// 아직도 Undetermined로 기각되지 않았다면 
			// 양 조합의 슬롯을 모두 사용했을 때 
			// 각 스킬의 달성 가능 최대 레벨이 얼마인지 비교한다
			// 어느 한쪽이 다른 쪽보다 항상 높으면 유리/불리를 따질 수 있다
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

					// 장식주를 꽂아서 달성 가능한 최대 레벨이 
					// 낮으면 한 쪽을 이용해서 다른 한 쪽을 조합할 수 없다
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
						// 아직 더 봐야 알 수 있다
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

	// 완전 동일 여부 체크
	bool exactSame = true;

	// 계산을 위해서 복제한다
	int slotLhs[3] = { lhs.slots[0], lhs.slots[1], lhs.slots[2], };
	int slotRhs[3] = { rhs.slots[0], rhs.slots[1], rhs.slots[2], };

	if (lhsSocketToUse >= 0 && lhsSocketToUse < COUNT_OF(slotLhs))
	{
		slotLhs[lhsSocketToUse]--;
	}

	bool notWorseThan[2] = { false, false };

	// 스킬의 차분을 먼저 구한다
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int lhsSkill = lhs.skills[i] + (i == lhsSkillToAdd ? 1 : 0);
		int rhsSkill = rhs.skills[i];

		// 완전 동일 여부를 확인한다
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

		// 어느 한 쪽에 남은 스킬을 
		// 반대쪽에 장식주를 달아서 커버 가능한지 확인한다
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
			// 장식주로 커버가 안 되는 스킬이다
			notWorseThan[index] = true;
			if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
		}
	}

	// 상호 슬롯을 비교한다
	if (lhs.SlotCount() > 0 || rhs.SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			// 완전 동일 여부를 확인한다
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

	// 완전 동일 여부 체크
	bool exactSame = true;

	// 계산을 위해서 복제한다
	bool notWorseThan[2] = { false, false };

	// 스킬의 차분을 먼저 구한다
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int lhsSkill = lhs.skills[i] + (i == lhsSkillToAdd ? 1 : 0);
		int rhsSkill = rhs.skills[i];

		// 완전 동일 여부를 확인한다
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

	// 상호 슬롯을 비교한다
	if (lhs.SlotCount() > 0 || rhs.SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			int lhsSlot = lhs.slots[slotSize] - (slotSize == lhsSocketToUse ? 1 : 0);
			int rhsSlot = rhs.slots[slotSize];

			// 완전 동일 여부를 확인한다
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

	// 완전 동일 여부 체크
	bool exactSame = true;

	// 계산을 위해서 복제한다
	int slotLhs[3] = { lhs.slots[0], lhs.slots[1], lhs.slots[2], };
	int slotRhs[3] = { rhs.slots[0], rhs.slots[1], rhs.slots[2], };

	bool notWorseThan[2] = { false, false };

	// 스킬의 차분을 먼저 구한다
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int lhsSkill = lhs.skills[i] + (i == lhsSkillToAdd ? 1 : 0);
		int rhsSkill = rhs.skills[i];

		// 완전 동일 여부를 확인한다
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

		// 어느 한 쪽에 남은 스킬을 
		// 반대쪽에 장식주를 달아서 커버 가능한지 확인한다
		if (g_skillToDecorator[i] != nullptr)
		{
			int slotSize = g_skillToDecorator[i]->slotSize;

			// 정확히 그 슬롯에 맞아야 한다
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
			// 장식주로 커버가 안 되는 스킬이다
			notWorseThan[index] = true;
			if (notWorseThan[0] && notWorseThan[1]) { return Undetermined; }
		}
	}

	// 상호 슬롯을 비교한다
	if (lhs.SlotCount() > 0 || rhs.SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			// 완전 동일 여부를 확인한다
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
	// 완전 동일 여부 체크
	bool exactSame = true;

	// 계산을 위해서 복제한다
	int slotRhs[3] = { rhs.slots[0], rhs.slots[1],rhs.slots[2], };

	// 스킬의 차분을 먼저 구한다
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int curSkill = skills[i];

		// 완전 동일 여부를 확인한다
		exactSame = exactSame && (curSkill == rhs.skills[i]);

		if (curSkill > 0)
		{
			int toSub = min(curSkill, rhs.skills[i]);
			curSkill = curSkill - toSub;

			// lhs에 남은 스킬을 rhs에 장식주를 달아서 커버 가능한지 확인한다
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
					// 장식주로 커버가 안 되는 스킬이다
					// 이 조합은 어쩔 수 없이 살려야 한다
					return NotWorse;
				}
			}
		}
	}

	// lhs의 슬롯에 꽂을 수 있는 장식주를
	// rhs의 슬롯에도 꽂을 수 있는지 확인한다
	if (SlotCount() > 0)
	{
		for (int slotSize = 0; slotSize < COUNT_OF(slots); ++slotSize)
		{
			int curSlot = slots[slotSize];

			// 완전 동일 여부를 확인한다
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
		// 완전 동일 여부를 확인한다
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
