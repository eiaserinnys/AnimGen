#pragma once

#include <list>

struct Armor;

struct GeneralizedCombination
{
	int* skills = nullptr;
	int skillCount = 0;

	int slots[3];

	GeneralizedCombination() = default;

	GeneralizedCombination(const GeneralizedCombination& rhs);

	GeneralizedCombination& operator = (const GeneralizedCombination& rhs);

	GeneralizedCombination(int skillCount);

	~GeneralizedCombination();

	void Combine(const GeneralizedCombination& rhs);

	int SlotCount() const;

	bool operator == (const GeneralizedCombination& rhs);

	bool operator <= (const GeneralizedCombination& rhs);

	void Dump() const;

	void DumpSimple() const;
};


struct GeneralizedArmor : public GeneralizedCombination
{
	typedef GeneralizedCombination ParentType;

	GeneralizedArmor(int skillCount);

	GeneralizedArmor(const GeneralizedArmor& rhs);

	GeneralizedArmor& operator = (const GeneralizedArmor& rhs);

	void Dump() const;

	std::list<Armor*> source;
};