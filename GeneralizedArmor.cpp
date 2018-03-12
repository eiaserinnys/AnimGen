#include "pch.h"
#include "GeneralizedArmor.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Armor.h"
#include "Decorator.h"

using namespace std;

//------------------------------------------------------------------------------
GeneralizedArmor::GeneralizedArmor(const GeneralizedArmor& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
GeneralizedArmor::GeneralizedArmor(int skillCount)
	: skillCount(skillCount)
{
	skills = new int[skillCount] { 0 };

	memset(slots, 0, sizeof(int) * COUNT_OF(slots));
}

//------------------------------------------------------------------------------
GeneralizedArmor::~GeneralizedArmor()
{
	delete[] skills;
}

//------------------------------------------------------------------------------
GeneralizedArmor& GeneralizedArmor::operator = (const GeneralizedArmor& rhs)
{
	delete[] skills;

	skillCount = rhs.skillCount;
	skills = new int[skillCount];

	memcpy(skills, rhs.skills, sizeof(int) * skillCount);

	memcpy(slots, rhs.slots, sizeof(int) * COUNT_OF(slots));

	source = rhs.source;

	return *this;
}

//------------------------------------------------------------------------------
int GeneralizedArmor::SlotCount() const
{
	return slots[0] + slots[1] + slots[2];
}

//------------------------------------------------------------------------------
bool GeneralizedArmor::operator == (const GeneralizedArmor& rhs)
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
bool GeneralizedArmor::operator <= (const GeneralizedArmor& rhs)
{
	// rhs에 장식주를 넣어서 
	// *this보다 유리한 상태가 되는지 확인한다
	GeneralizedArmor populated = rhs;

	bool result = false;

	function<void(int)> Populate;

	Populate = [&](int slot)
	{
		if (result) { return; }

		// 매 단계마다 체크한다
		bool lessThan = true;
		for (int i = 0; i < skillCount; ++i)
		{
			if (skills[i] > populated.skills[i]) { lessThan = false; break; }
		}

		if (lessThan)
		{
			// 스킬이 포함되는 경우 슬롯도 그런지 확인한다
			if (SlotCount() > 0)
			{
				int backupLhs[3];
				int backupRhs[3];

				memcpy(backupLhs, slots, sizeof(int) * COUNT_OF(slots));
				memcpy(backupRhs, populated.slots, sizeof(int) * COUNT_OF(slots));

				bool failed = false;

				// lhs의 슬롯에 꽂을 수 있는 장식주를
				// rhs의 슬롯에도 꽂을 수 있는지 확인한다
				int slotSize = 0;
				while (slotSize < COUNT_OF(slots) && lessThan)
				{
					if (backupLhs[slotSize] > 0)
					{
						bool selected = false;
						for (int i = slotSize; i < COUNT_OF(backupLhs); ++i)
						{
							if (backupRhs[i] > 0)
							{
								backupLhs[slotSize]--;
								backupRhs[i]--;
								selected = true;
								break;
							}
						}
						if (!selected)
						{
							lessThan = false;
						}
					}
					else
					{
						slotSize++;
					}
				}
			}
		}

		if (lessThan)
		{
			WindowsUtility::Debug(L"Dropping '");
			DumpSimple();
			WindowsUtility::Debug(L"' for '");
			populated.DumpSimple();
			WindowsUtility::Debug(L"' from '");
			rhs.DumpSimple();
			WindowsUtility::Debug(L"'.\n");

			result = true;
			return;
		}

		// 빈 슬롯에 장식주를 넣는다
		if (slot < rhs.SlotCount())
		{
			for (int j = 0; j < COUNT_OF(populated.slots); ++j)
			{
				if (populated.slots[j] > 0)
				{
					for (int i = 0; i < g_decorators.size(); ++i)
					{
						auto dec = g_decorators[i];
						if (dec->slotSize <= j + 1)
						{
							populated.slots[j]--;
							populated.skills[dec->skillIndex]++;

							Populate(slot + 1);

							populated.skills[dec->skillIndex]--;
							populated.slots[j]++;
						}
					}
				}
			}
		}
	};

	Populate(0);

	return result;
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

//------------------------------------------------------------------------------
void GeneralizedArmor::DumpSimple() const
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
