#pragma once

#include <string>
#include <vector>
#include <map>

//------------------------------------------------------------------------------
struct Set
{
	std::wstring name;
	int rarity;
	int defense;
	std::wstring skill;
};

//------------------------------------------------------------------------------
struct Armor
{
	enum PartType
	{
		Head,
		Body,
		Arm,
		Waist,
		Leg,

		Count,
	};

	bool IsRelevant() const;

	void Dump(bool addNewLine) const;

	Set*				set;
	PartType			type;
	std::wstring		name;
	std::vector<std::pair<std::wstring, int>>	
						skills;
	std::vector<int>	slots;
};

//------------------------------------------------------------------------------
extern std::vector<Set*> g_sets;

extern std::map<Armor::PartType, std::vector<Armor*>*> g_armors;

void LoadArmors(bool dump);

