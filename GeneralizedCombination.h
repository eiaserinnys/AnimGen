#pragma once

#include <list>

#include "CombinationBase.h"

struct Armor;
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

	void DeriveFrom(
		const GeneralizedCombination* prev);

	void Combine(
		const GeneralizedCombination* prev,
		const Charm* charm);

	void Combine(
		const GeneralizedCombination* prev, 
		int partIndex, 
		const GeneralizedArmor* part);

	void CombineEquivalent(GeneralizedCombination* rhs);

	void Delete() { delete this; }

	void Dump() const;

	std::list<PartInstance*> instances;

	std::list<GeneralizedCombination*> equivalents;

	void ClearInstances();
};

//------------------------------------------------------------------------------
struct Decorator;

struct DecoratedCombination : public CombinationBase
{
	typedef CombinationBase ParentType;

	DecoratedCombination();

	static DecoratedCombination* DeriveFrom(
		const GeneralizedCombination* comb);

	static DecoratedCombination* DeriveFrom(
		DecoratedCombination* comb);

	static DecoratedCombination* DeriveFrom(
		DecoratedCombination* comb, 
		const Decorator* dec, 
		int socket,
		int decIndex);

	void CombineEquivalent(DecoratedCombination* rhs);

	void Delete();

	void Write(FILE* file) const;

public:
	const GeneralizedCombination* source = nullptr;

	std::list<DecoratedCombination*> equivalents;

	const Decorator* decorator = nullptr;
	int lastSocket = -1;
	int lastDecoratorIndex = 0;

private:
	DecoratedCombination* derivedFrom = nullptr;
	~DecoratedCombination();
	int refCount = 1;
};
