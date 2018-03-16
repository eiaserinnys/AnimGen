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
static void CalculateFreedom(
	const EvaluatingSkills& evSkills,
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

		for (auto si : evSkills.bySlotSize[slotSize])
		{
			int lvDiff = evSkills.list[si].maxLevel - side[s]->skills[si];

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
		const EvaluatingSkills& evSkills,
		const CombinationBase& lhs,
		const CombinationBase& rhs)
{
	return CompareStrict2(evSkills, lhs, rhs, -1, -1);
}

//------------------------------------------------------------------------------
CombinationBase::ComparisonResult
	CombinationBase::CompareStrict2(
		const EvaluatingSkills& evSkills,
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
	for (auto si : evSkills.noDecorator)
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
	assert(evSkills.list.size() < bufferSize);

	if (skillToAddToLhs >= 0) { skills[0][skillToAddToLhs]++; }

	for (int i = 0; i < COUNT_OF(side); ++i)
	{
		memcpy(skills[i], side[i]->skills, sizeof(int) * evSkills.list.size());
	}

	// 슬롯별 자유도를 바탕으로 상호 표현 가능성을 검증한다
	for (int slotSize = 0; slotSize < 3; ++slotSize)
	{
		// 먼저 슬롯 크기별 자유도를 계산한다
		int freedom[] = { 0, 0, };
		bool maxed[] = { false, false, };
		CalculateFreedom(evSkills, side, slotSize, lhsSocketToUse, freedom, maxed);

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

			for (auto si : evSkills.bySlotSize[slotSize])
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

				for (auto si : evSkills.bySlotSize[slotSize])
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
			assert(evSkills.bySlotSize[slotSize].size() < bufferSize);

			function<bool()> EvaluatePermutations;

			EvaluatePermutations = [&]() -> bool
			{
				for (auto si : evSkills.bySlotSize[slotSize])
				{
					if (checked[si] != 0) { continue; }

					if (skills[0][si] >= evSkills.list[si].maxLevel &&
						skills[1][si] >= evSkills.list[si].maxLevel)
					{
						continue;
					}

					int toUse[] =
					{
						min(evSkills.list[si].maxLevel - skills[0][si], available[0]),
						min(evSkills.list[si].maxLevel - skills[1][si], available[1]),
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
void CombinationBase::Combine(
	const EvaluatingSkills& evSkills,
	const CombinationBase& rhs)
{
	for (int i = 0; i < skillCount; ++i)
	{
		skills[i] += rhs.skills[i];

		if (skills[i] > evSkills.list[i].maxLevel)
		{
			skills[i] = evSkills.list[i].maxLevel;
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
void CombinationBase::Dump(const EvaluatingSkills& evSkills) const
{
	DumpSimple(evSkills);

	WindowsUtility::Debug(L"\n");
}

//------------------------------------------------------------------------------
void CombinationBase::Dump(const EvaluatingSkills& evSkills, FILE* file) const
{
	fwprintf(file, L"%s\n", DumpToString(evSkills).c_str());
}

//------------------------------------------------------------------------------
wstring CombinationBase::DumpToString(const EvaluatingSkills& evSkills) const
{
	wstring result;

	bool first = true;
	for (int i = 0; i < skillCount; ++i)
	{
		if (skills[i] > 0)
		{
			result += Utility::FormatW(
				first ? L"%s%d" : L" %s%d",
				evSkills.list[i].abbName.c_str(),
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
void CombinationBase::DumpSimple(const EvaluatingSkills& evSkills) const
{
	bool first = true;
	for (int i = 0; i < skillCount; ++i)
	{
		if (skills[i] > 0)
		{
			WindowsUtility::Debug(
				first ? L"%s%d" : L" %s%d",
				evSkills.list[i].abbName.c_str(),
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
