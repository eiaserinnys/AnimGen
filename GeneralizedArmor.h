#include "Armor.h"

#include "GeneralizedCombination.h"

//------------------------------------------------------------------------------
struct GeneralizedArmor : public CombinationBase
{
	typedef CombinationBase ParentType;

	GeneralizedArmor(const Armor* armor);

	GeneralizedArmor(const GeneralizedArmor& rhs);

	GeneralizedArmor& operator = (const GeneralizedArmor& rhs);

	void Dump() const;

	void Dump(FILE* file) const;

	void CombineEquivalent(const GeneralizedArmor& rhs);

	std::list<const Armor*> source;
};

//------------------------------------------------------------------------------
void FilterArmors(
	std::map<Armor::PartType, std::list<GeneralizedArmor*>*>& g_generalized,
	bool dumpList, 
	bool dumpComparison);

