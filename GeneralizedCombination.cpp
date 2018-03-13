#include "pch.h"
#include "GeneralizedCombination.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Armor.h"
#include "Decorator.h"

using namespace std;

//------------------------------------------------------------------------------
GeneralizedCombinationBase::GeneralizedCombinationBase(const GeneralizedCombinationBase& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
GeneralizedCombinationBase::GeneralizedCombinationBase(int skillCount)
	: skillCount(skillCount)
{
	skills = new int[skillCount] { 0 };

	memset(slots, 0, sizeof(int) * COUNT_OF(slots));
}

//------------------------------------------------------------------------------
GeneralizedCombinationBase::~GeneralizedCombinationBase()
{
	delete[] skills;
}

//------------------------------------------------------------------------------
GeneralizedCombinationBase& GeneralizedCombinationBase::operator = (const GeneralizedCombinationBase& rhs)
{
	delete[] skills;

	skillCount = rhs.skillCount;
	skills = new int[skillCount];

	memcpy(skills, rhs.skills, sizeof(int) * skillCount);

	memcpy(slots, rhs.slots, sizeof(int) * COUNT_OF(slots));

	return *this;
}

//------------------------------------------------------------------------------
int GeneralizedCombinationBase::SlotCount() const
{
	return slots[0] + slots[1] + slots[2];
}

//------------------------------------------------------------------------------
GeneralizedCombinationBase::ComparisonResult 
	GeneralizedCombinationBase::Compare(
		const GeneralizedCombinationBase& rhs) const
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
			exactSame = exactSame && (curSlot == slots[slotSize]);

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
bool GeneralizedCombinationBase::IsWorseThanOrEqualTo(const GeneralizedCombinationBase& rhs) const
{
	auto result = Compare(rhs);
	return result != NotWorse;
}

//------------------------------------------------------------------------------
void GeneralizedCombinationBase::Combine(const GeneralizedCombinationBase& rhs)
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
void GeneralizedCombinationBase::Dump() const
{
	DumpSimple();

	WindowsUtility::Debug(L"\n");
}

//------------------------------------------------------------------------------
void GeneralizedCombinationBase::DumpSimple() const
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

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
GeneralizedCombination::GeneralizedCombination(int skillCount)
	: ParentType(skillCount)
{
}

//------------------------------------------------------------------------------
GeneralizedCombination::~GeneralizedCombination()
{
	ClearInstances();
}

//------------------------------------------------------------------------------
void GeneralizedCombination::ClearInstances()
{
	for (auto comb : instances) { delete comb; }
	instances.clear();
}

//------------------------------------------------------------------------------
GeneralizedCombination::GeneralizedCombination(const GeneralizedCombination& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
GeneralizedCombination& GeneralizedCombination::operator = (const GeneralizedCombination& rhs)
{
	ParentType::operator = (rhs);

	for (auto inst : rhs.instances)
	{
		auto newInst = new PartInstance;
		*newInst = *inst;
		instances.push_back(newInst);
	}

	return *this;
}

//------------------------------------------------------------------------------
void GeneralizedCombination::Combine(
	const GeneralizedCombination* prev,
	int partIndex, 
	const GeneralizedArmor* part)
{
	ClearInstances();

	if (prev != nullptr)
	{ 
		ParentType::operator = (*prev);
	}

	ParentType::Combine(*part);

	if (prev != nullptr)
	{
		for (auto prevInst : prev->instances)
		{
			auto inst = new PartInstance;
			*inst = *prevInst;
			
			inst->parts[partIndex] = part;

			instances.push_back(inst);
		}
	}
	else
	{
		auto inst = new PartInstance;
		inst->parts[partIndex] = part;
		instances.push_back(inst);
	}
}

//------------------------------------------------------------------------------
void GeneralizedCombination::CombineEquivalent(GeneralizedCombination* target)
{
	instances.insert(
		instances.end(),
		target->instances.begin(),
		target->instances.end());

	target->instances.clear();
}

//------------------------------------------------------------------------------
void GeneralizedCombination::Dump() const
{
	DumpSimple();

	WindowsUtility::Debug(L"\n");

	for (auto it = instances.begin(); it != instances.end(); ++it)
	{
		auto inst = *it;

		WindowsUtility::Debug(L"\t");

		for (int i = 0; i < COUNT_OF(inst->parts); ++i)
		{
			if (inst->parts[i] == nullptr) { continue; }

			WindowsUtility::Debug(
				L"%s", 
				(*inst->parts[i]->source.begin())->name.c_str());

			if (inst->parts[i]->source.size() > 1)
			{
				WindowsUtility::Debug(L"(�� %d��) | ", inst->parts[i]->source.size());
			}
			else
			{
				WindowsUtility::Debug(L" | ");
			}
		}

		WindowsUtility::Debug(L"\n");
	}
}

////////////////////////////////////////////////////////////////////////////////

int instance = 0;

//------------------------------------------------------------------------------
void DecoratedCombination::Delete()
{
	refCount--;

	assert(refCount >= 0);

	if (refCount == 0) 
	{ 
		if (derivedFrom != nullptr)
		{
			derivedFrom->Delete();
			derivedFrom = nullptr;
		}
		delete this; 
	}
}

//------------------------------------------------------------------------------
DecoratedCombination::DecoratedCombination()
{
	instance++;
}

//------------------------------------------------------------------------------
DecoratedCombination* DecoratedCombination::DeriveFrom(
	const GeneralizedCombination* comb)
{
	auto ret = new DecoratedCombination;

	(*ret).ParentType::operator = (*comb);

	ret->source = comb;

	return ret;
}

//------------------------------------------------------------------------------
DecoratedCombination* DecoratedCombination::DeriveFrom(
	DecoratedCombination* from)
{
	auto ret = new DecoratedCombination;

	(*ret).ParentType::operator = (*from);

	ret->source = from->source;

	ret->derivedFrom = from;

	from->refCount++;

	return ret;
}

//------------------------------------------------------------------------------
DecoratedCombination::~DecoratedCombination()
{
	if (addedAsEquivalent)
	{
		int i = 0;
	}

	assert(refCount == 0);

	for (auto e : equivalents) 
	{ 
		e->Delete(); 
	}
	equivalents.clear();

	instance--;
}

//------------------------------------------------------------------------------
void DecoratedCombination::CombineEquivalent(DecoratedCombination* rhs)
{
	equivalents.push_back(rhs);
	equivalents.insert(equivalents.end(), rhs->equivalents.begin(), rhs->equivalents.end());
	rhs->equivalents.clear();
}
