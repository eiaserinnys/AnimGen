#pragma once

#include <list>

struct Armor;

//------------------------------------------------------------------------------
struct GeneralizedCombinationBase
{
	int* skills = nullptr;
	int skillCount = 0;

	int slots[3];

	GeneralizedCombinationBase() = default;

	GeneralizedCombinationBase(const GeneralizedCombinationBase& rhs);

	GeneralizedCombinationBase& operator = (const GeneralizedCombinationBase& rhs);

	GeneralizedCombinationBase(int skillCount);

	~GeneralizedCombinationBase();

	void Combine(const GeneralizedCombinationBase& rhs);

	int SlotCount() const;

	enum ComparisonResult
	{
		NotWorse,
		Worse,
		Equal, 
	};

	ComparisonResult Compare(const GeneralizedCombinationBase& rhs) const;

	bool IsWorseThanOrEqualTo(const GeneralizedCombinationBase& rhs) const;

	void Dump() const;

	void DumpSimple() const;
};

//------------------------------------------------------------------------------
struct GeneralizedArmor : public GeneralizedCombinationBase
{
	typedef GeneralizedCombinationBase ParentType;

	GeneralizedArmor(int skillCount);

	GeneralizedArmor(const GeneralizedArmor& rhs);

	GeneralizedArmor& operator = (const GeneralizedArmor& rhs);

	void Dump() const;

	std::list<Armor*> source;
};

//------------------------------------------------------------------------------
struct CombinationInstance
{
	const GeneralizedArmor* parts[5] = { nullptr, nullptr, nullptr, nullptr, nullptr, };
};

struct GeneralizedCombination : public GeneralizedCombinationBase
{
	typedef GeneralizedCombinationBase ParentType;

	~GeneralizedCombination();

	GeneralizedCombination(int skillCount);

	GeneralizedCombination(const GeneralizedCombination& rhs);

	GeneralizedCombination& operator = (const GeneralizedCombination& rhs);

	void Combine(
		const GeneralizedCombination* prev, 
		int partIndex, 
		const GeneralizedArmor* part);

	void Dump() const;

	std::list<CombinationInstance*> instances;

	void ClearInstances();
};

