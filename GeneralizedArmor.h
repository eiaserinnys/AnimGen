#include "Armor.h"

#include "Skill.h"
#include "GeneralizedCombination.h"

//------------------------------------------------------------------------------
struct GeneralizedArmor : public CombinationBase
{
	typedef CombinationBase ParentType;

	GeneralizedArmor(const EvaluatingSkills& evSkills, const Armor* armor);

	GeneralizedArmor(const GeneralizedArmor& rhs);

	GeneralizedArmor& operator = (const GeneralizedArmor& rhs);

	void Dump(const EvaluatingSkills& evSkills) const;

	void Dump(const EvaluatingSkills& evSkills, FILE* file) const;

	void CombineEquivalent(const GeneralizedArmor& rhs);

	std::list<const Armor*> source;
};

//------------------------------------------------------------------------------
void FilterArmors(
	const EvaluatingSkills& evSkills,
	std::map<Armor::PartType, std::list<GeneralizedArmor*>*>& g_generalized,
	bool dumpList, 
	bool dumpComparison);

