#include "pch.h"
#include "GeneralizedCombination.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Armor.h"
#include "Decorator.h"
#include "Charm.h"

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
	const GeneralizedCombinationBase& lhs,
	const GeneralizedCombinationBase& rhs)
{
	return Compare(lhs, rhs, -1, -1);
}

//------------------------------------------------------------------------------
GeneralizedCombinationBase::ComparisonResult
	GeneralizedCombinationBase::Compare(
		const GeneralizedCombinationBase& lhs, 
		const GeneralizedCombinationBase& rhs,
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

		int toSub = min(lhsSkill , rhsSkill);
		lhsSkill = lhsSkill - toSub;
		rhsSkill = rhsSkill - toSub;

		if (lhsSkill == 0 && rhsSkill == 0) { continue; }

		if (lhsSkill > 0) { assert(rhsSkill == 0); } else { assert(lhsSkill == 0); }

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
GeneralizedCombinationBase::ComparisonResult 
	GeneralizedCombinationBase::Compare(
		const GeneralizedCombinationBase& rhs) const
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
				WindowsUtility::Debug(L"(외 %d개) | ", inst->parts[i]->source.size());
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
	DecoratedCombination* from,
	const Decorator* dec,
	int socket,
	int decIndex)
{
	auto ret = new DecoratedCombination;

	(*ret).ParentType::operator = (*from);

	ret->source = from->source;

	ret->derivedFrom = from;

	from->refCount++;

	if (dec != nullptr)
	{
		auto index = dec->skillIndex;

		if (ret->skills[index] + 1 <= g_skillMaxLevel[index])
		{
			ret->skills[index]++;
			ret->decorator = dec;
		}

		ret->lastSocket = socket;
		ret->lastDecoratorIndex = decIndex;

		ret->slots[socket]--;	// 어쨌든 슬롯은 소비
	}

	return ret;
}

//------------------------------------------------------------------------------
DecoratedCombination* DecoratedCombination::DeriveFrom(
	DecoratedCombination* from,
	const Charm* charm)
{
	auto ret = new DecoratedCombination;

	(*ret).ParentType::operator = (*from);

	ret->source = from->source;

	ret->derivedFrom = from;

	from->refCount++;

	if (charm != nullptr)
	{
		auto index = charm->skillIndex;

		int total = ret->skills[index] + charm->skillLevel;

		ret->skills[index] = 
			 total < g_skillMaxLevel[index] ? total : g_skillMaxLevel[index];
		ret->charm = charm;
	}

	return ret;
}

//------------------------------------------------------------------------------
DecoratedCombination::~DecoratedCombination()
{
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

//------------------------------------------------------------------------------
void DecoratedCombination::Write(FILE* file) const
{
	// 첫번째 조합만 덤프하자
	auto inst = *source->instances.begin();
	for (int i = 0; i < COUNT_OF(inst->parts); ++i)
	{
		fwprintf(file, L"%s ", (*inst->parts[i]->source.begin())->name.c_str());
	}

	if (charm != nullptr)
	{
		fwprintf(file, L"%s ", charm->name.c_str());
	}

	const DecoratedCombination* c = this;
	while (c != nullptr)
	{
		if (c->decorator != nullptr)
		{
			fwprintf(file, L"%s ", c->decorator->name.c_str());
		}

		c = c->derivedFrom;
	}
}