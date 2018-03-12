#pragma once

#include <string>
#include <vector>

struct Set
{
	std::wstring name;
	int rarity;
	int defense;
	std::wstring skill;
};

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

	Set*				set;
	PartType			type;
	std::wstring		name;
	std::vector<std::pair<std::wstring, int>>	
						skills;
	std::vector<int>	slots;
};
