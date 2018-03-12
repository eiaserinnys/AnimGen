#include "pch.h"
#include "GeneralizedCombination.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Armor.h"
#include "Decorator.h"

using namespace std;

// 싱글 스레드 가정 핵
static int skillLhs[1024];
static int skillRhs[1024];

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
	memcpy(&skillLhs[0], skills, sizeof(int) * skillCount);
	memcpy(&skillRhs[0], rhs.skills, sizeof(int) * skillCount);

	int slotLhs[3];
	int slotRhs[3];

	memcpy(slotLhs, slots, sizeof(int) * COUNT_OF(slots));
	memcpy(slotRhs, rhs.slots, sizeof(int) * COUNT_OF(slots));

	// 스킬의 차분을 먼저 구한다
	int skillLeft = 0;
	for (int i = 0; i < skillCount; ++i)
	{
		int toSub = min(skillLhs[i], skillRhs[i]);
		skillLhs[i] -= toSub;
		skillRhs[i] -= toSub;

		skillLeft += skillLhs[i];
	}

	// lhs에 남은 스킬을 rhs에 장식주를 달아서 커버 가능한지 확인한다
	if (skillLeft > 0)
	{
		for (int i = 0; i < skillCount; ++i)
		{
			while (skillLhs[i] > 0)
			{
				if (g_skillToDecorator[i] != nullptr)
				{
					int slotSize = g_skillToDecorator[i]->slotSize;

					bool slotFound = false;

					for (int j = slotSize - 1; j < 3; ++j)
					{
						int toSub = min(skillLhs[i], slotRhs[j]);

						if (toSub > 0)
						{
							slotRhs[j] -= toSub;
							skillLhs[i] -= toSub;
							slotFound = true;
							break;
						}
					}

					if (!slotFound)
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
	}

	// lhs의 슬롯에 꽂을 수 있는 장식주를
	// rhs의 슬롯에도 꽂을 수 있는지 확인한다
	if (slotLhs[0] + slotLhs[1] + slotLhs[2] > 0)
	{
		int slotSize = 0;
		while (slotSize < COUNT_OF(slots))
		{
			if (slotLhs[slotSize] > 0)
			{
				bool selected = false;
				for (int i = slotSize; i < COUNT_OF(slotLhs); ++i)
				{
					if (slotRhs[i] > 0)
					{
						slotLhs[slotSize]--;
						slotRhs[i]--;
						selected = true;
						break;
					}
				}
				if (!selected)
				{
					return false;
				}
			}
			else
			{
				slotSize++;
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
