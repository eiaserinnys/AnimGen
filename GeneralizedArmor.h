#pragma once

#include <list>

struct Armor;

struct GeneralizedArmor
{
	int* skills = nullptr;
	int skillCount = 0;

	int slots[3];

	std::list<Armor*> source;

	GeneralizedArmor() = default;

	GeneralizedArmor(const GeneralizedArmor& rhs);

	GeneralizedArmor& operator = (const GeneralizedArmor& rhs);

	GeneralizedArmor(int skillCount);

	~GeneralizedArmor();

	int SlotCount() const;

	bool operator == (const GeneralizedArmor& rhs);

	bool operator <= (const GeneralizedArmor& rhs);

	void Dump() const;

	void DumpSimple() const;
};
