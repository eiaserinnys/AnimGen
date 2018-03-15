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

	void CombineSource(const GeneralizedArmor& rhs);

	std::list<const Armor*> source;
};

//------------------------------------------------------------------------------
void FilterArmors(
	std::map<Armor::PartType, std::vector<GeneralizedArmor*>*>& g_generalized,
	bool dump);