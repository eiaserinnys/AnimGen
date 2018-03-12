#include "pch.h"
#include "GeneralizedCombination.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Armor.h"
#include "Decorator.h"

using namespace std;

//------------------------------------------------------------------------------
GeneralizedCombination::GeneralizedCombination(const GeneralizedCombination& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
GeneralizedCombination::GeneralizedCombination(int skillCount)
	: skillCount(skillCount)
{
	skills = new int[skillCount] { 0 };

	memset(slots, 0, sizeof(int) * COUNT_OF(slots));
}

//------------------------------------------------------------------------------
GeneralizedCombination::~GeneralizedCombination()
{
	delete[] skills;
}

//------------------------------------------------------------------------------
GeneralizedCombination& GeneralizedCombination::operator = (const GeneralizedCombination& rhs)
{
	delete[] skills;

	skillCount = rhs.skillCount;
	skills = new int[skillCount];

	memcpy(skills, rhs.skills, sizeof(int) * skillCount);

	memcpy(slots, rhs.slots, sizeof(int) * COUNT_OF(slots));

	return *this;
}

//------------------------------------------------------------------------------
int GeneralizedCombination::SlotCount() const
{
	return slots[0] + slots[1] + slots[2];
}

//------------------------------------------------------------------------------
bool GeneralizedCombination::operator == (const GeneralizedCombination& rhs)
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
bool GeneralizedCombination::operator <= (const GeneralizedCombination& rhs)
{
	// 계산을 위해서 복제한다
	int slotRhs[3] = { rhs.slots[0], rhs.slots[1],rhs.slots[2], };

	// 스킬의 차분을 먼저 구한다
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int curSkill = skills[i];

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
					return false;
				}
			}
			else
			{
				// 장식주로 커버가 안 되는 스킬이다
				// 이 조합은 어쩔 수 없이 살려야 한다
				return false;
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

			if (curSlot <= 0) { continue; }

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
				return false;
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void GeneralizedCombination::Combine(const GeneralizedCombination& rhs)
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
void GeneralizedCombination::Dump() const
{
	DumpSimple();

	WindowsUtility::Debug(L"\n");
}

//------------------------------------------------------------------------------
void GeneralizedCombination::DumpSimple() const
{
	bool first = true;
	for (int i = 0; i < skillCount; ++i)
	{
		if (skills[i] > 0)
		{
			WindowsUtility::Debug(
				first ? L"%s Lv.%d" : L" %s Lv.%d",
				g_skills[i].c_str(),
				skills[i]);
			first = false;
		}
	}

	for (int i = 0; i < COUNT_OF(slots); ++i)
	{
		for (int j = 0; j < slots[i]; ++j)
		{
			WindowsUtility::Debug(L" [%d]", i + 1);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
GeneralizedArmor::GeneralizedArmor(int skillCount)
	: ParentType(skillCount)
{
}

//------------------------------------------------------------------------------
GeneralizedArmor::GeneralizedArmor(const GeneralizedArmor& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
GeneralizedArmor& GeneralizedArmor::operator = (const GeneralizedArmor& rhs)
{
	ParentType::operator = (rhs);

	source = rhs.source;

	return *this;
}

//------------------------------------------------------------------------------
void GeneralizedArmor::Dump() const
{
	DumpSimple();

	WindowsUtility::Debug(L"\n");

	for (auto it = source.begin(); it != source.end(); ++it)
	{
		WindowsUtility::Debug(L"\t%s\n", (*it)->name.c_str());
	}
}
