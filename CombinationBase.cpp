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

	// Strict = �� ���� �ڸ����� �ش� ũ���� ����ָ� ����ٰ� ����

	// ���� ����ַ� �ذ���� �ʴ� ��ų�� ���Ѵ�
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

	// ��ų ���¸� �����Ѵ�
	const int bufferSize = 32;
	int skills[2][bufferSize];
	assert(evSkills.list.size() < bufferSize);

	if (skillToAddToLhs >= 0) { skills[0][skillToAddToLhs]++; }

	for (int i = 0; i < COUNT_OF(side); ++i)
	{
		memcpy(skills[i], side[i]->skills, sizeof(int) * evSkills.list.size());
	}

	// ���Ժ� �������� �������� ��ȣ ǥ�� ���ɼ��� �����Ѵ�
	for (int slotSize = 0; slotSize < 3; ++slotSize)
	{
		// ���� ���� ũ�⺰ �������� ����Ѵ�
		int freedom[] = { 0, 0, };
		bool maxed[] = { false, false, };
		CalculateFreedom(evSkills, side, slotSize, lhsSocketToUse, freedom, maxed);

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

			for (auto si : evSkills.bySlotSize[slotSize])
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

				// ����
				available[s] = freedom[s];
			}

			// ������ Undetermined�� �Ⱒ���� �ʾҴٸ� 
			// �� ������ ������ ��� ������� �� 
			// �� ��ų�� �޼� ���� �ִ� ������ ������ ���Ѵ�
			// ��� ������ �ٸ� �ʺ��� �׻� ������ ����/�Ҹ��� ���� �� �ִ�
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
