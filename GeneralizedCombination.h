#pragma once

#include <list>

#include "CombinationBase.h"
#include "Armor.h"

struct Charm;
struct GeneralizedArmor;

struct PartInstance
{
	const Charm* charm = nullptr;
	const GeneralizedArmor* parts[5] = { nullptr, nullptr, nullptr, nullptr, nullptr, };
};

struct GeneralizedCombination : public CombinationBase
{
	typedef CombinationBase ParentType;

	~GeneralizedCombination();

	GeneralizedCombination(int skillCount);

	GeneralizedCombination(const GeneralizedCombination& rhs);

	GeneralizedCombination& operator = (const GeneralizedCombination& rhs);

	void Combine(
		const GeneralizedCombination* prev,
		const Charm* charm);

	void Combine(
		const GeneralizedCombination* prev, 
		int partIndex, 
		const GeneralizedArmor* part);

	void CombineEquivalent(GeneralizedCombination& rhs);

	void Delete() { delete this; }

	void Dump() const;

	void Dump(FILE* file) const;

	std::list<PartInstance*> instances;

	std::list<GeneralizedCombination*> equivalents;

	void ClearInstances();
};

void PopulateArmors(
	const std::map<Armor::PartType, std::list<GeneralizedArmor*>*>& g_generalized,
	std::list<GeneralizedCombination*>& all,
	bool dumpList,
	bool dumpComparison);

