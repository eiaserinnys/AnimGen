#pragma once

#include <list>

#include "CombinationBase.h"
#include "GeneralizedCombination.h"

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

	void CombineEquivalent(DecoratedCombination& rhs);

	void Delete();

	void Write(FILE* file) const;

public:
	const GeneralizedCombination* source = nullptr;

	std::list<DecoratedCombination*> equivalents;

	std::list<const Decorator*> decorator;

private:
	DecoratedCombination* derivedFrom = nullptr;
	~DecoratedCombination();
	int refCount = 1;
};

class IDamageCalculator {
public:
	virtual ~IDamageCalculator() {}
	virtual double Do(const CombinationBase* comb) const = 0;
};

void PopulateDecorators(
	const EvaluatingSkills& evSkills,
	const std::list<GeneralizedCombination*>& g_all,
	std::list<DecoratedCombination*>& g_decAll,
	IDamageCalculator* damageCalc,
	int maxCount, 
	bool dumpComparison);