#include "pch.h"
#include "GeneralizedCombination.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Armor.h"
#include "Decorator.h"
#include "Charm.h"

#include "GeneralizedArmor.h"

using namespace std;

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
	const Charm* charm)
{
	ClearInstances();

	if (prev != nullptr)
	{
		ParentType::operator = (*prev);
	}

	for (int i = 0; i < charm->skills.size(); ++i)
	{
		int index = charm->skills[i].second;

		skills[index] += charm->skillLevel;

		if (skills[index] > g_skillMaxLevel[index])
		{
			skills[index] = g_skillMaxLevel[index];
		}
	}

	if (prev != nullptr)
	{
		for (auto prevInst : prev->instances)
		{
			auto inst = new PartInstance;
			*inst = *prevInst;
			inst->charm = charm;
			instances.push_back(inst);
		}
	}
	else
	{
		auto inst = new PartInstance;
		inst->charm = charm;
		instances.push_back(inst);
	}
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

		if (inst->charm != nullptr)
		{
			WindowsUtility::Debug(L"%s", inst->charm->name.c_str());
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
	return DeriveFrom(from, nullptr, -1, -1);
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

		ret->lastDecoratorIndex = decIndex;
	}

	if (socket >= 0)
	{
		ret->slots[socket]--;	// 어쨌든 슬롯은 소비
		ret->lastSocket = socket;
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

	if (inst->charm != nullptr)
	{
		fwprintf(file, L"%s ", inst->charm->name.c_str());
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