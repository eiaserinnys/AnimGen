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
void GeneralizedCombination::CombineEquivalent(GeneralizedCombination& target)
{
	instances.insert(
		instances.end(),
		target.instances.begin(),
		target.instances.end());

	target.instances.clear();
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

//------------------------------------------------------------------------------
void GeneralizedCombination::Dump(FILE* file) const
{
	ParentType::Dump(file);

	for (auto it = instances.begin(); it != instances.end(); ++it)
	{
		auto inst = *it;

		fwprintf(file, L"\t");

		for (int i = 0; i < COUNT_OF(inst->parts); ++i)
		{
			if (inst->parts[i] == nullptr) { continue; }

			fwprintf(file, 
				L"%s",
				(*inst->parts[i]->source.begin())->name.c_str());

			if (inst->parts[i]->source.size() > 1)
			{
				fwprintf(file, L"(외 %d개) | ", (int) inst->parts[i]->source.size());
			}
			else
			{
				fwprintf(file, L" | ");
			}
		}

		if (inst->charm != nullptr)
		{
			fwprintf(file, L"%s", inst->charm->name.c_str());
		}

		fwprintf(file, L"\n");
	}
}

//------------------------------------------------------------------------------
void PopulateArmors(
	const map<Armor::PartType, list<GeneralizedArmor*>*>& g_generalized,
	list<GeneralizedCombination*>& g_all,
	bool dumpList,
	bool dumpComparison)
{
	FILE* file;
	fopen_s(&file, "log_combination.txt", "w,ccs=UNICODE");

	// 초기 리스트를 만든다
	{
		for (int i = 0; i < g_charms.size(); ++i)
		{
			auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
			newComb->Combine(nullptr, g_charms[i]);
			g_all.push_back(newComb);
		}

		fwprintf(file, L"%d combinations with charms ---------------------------------------\n", (int)g_all.size());
		WindowsUtility::Debug(L"%d combinations with charms ---------------------------------------\n", (int)g_all.size());

		// 전체 덤프 ㄱㄱ
		if (dumpList) { for (auto g : g_all) { g->Dump(file); } }
	}

	// 다음 채널을 추가해서 늘린다
	for (int channel = 0; channel < Armor::Count; ++channel)
	{
		list<GeneralizedCombination*> next;

		auto it = g_generalized.find((Armor::PartType) channel);
		if (it == g_generalized.end())
		{
			throw invalid_argument("");
		}

		auto& gs = *it->second;

		// 모든 조합 x 조합의 페어를 일단 생성
		for (auto it = g_all.begin(); it != g_all.end(); ++it)
		{
			for (auto g : gs)
			{
				auto newComb = new GeneralizedCombination(COUNT_OF(g_skills));
				newComb->Combine(*it, channel, g);

				AddIfNotWorse(next, newComb, true, file, dumpComparison);
			}
		}

		// 전체 덤프 ㄱㄱ
		for (auto g : g_all) { delete g; }

		g_all.swap(next);

		fwprintf(file, L"%d combinations until part %d ---------------------------------------\n", (int)g_all.size(), channel + 1);
		WindowsUtility::Debug(L"%d combinations until part %d ---------------------------------------\n", g_all.size(), channel + 1);
		if (dumpList) { for (auto g : g_all) { g->Dump(file); } }
	}

	fclose(file);
}
