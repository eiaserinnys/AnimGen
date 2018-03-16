#pragma once

#include <string>

#include "Skill.h"

struct Decorator
{
	std::wstring	name;
	std::wstring	skill;
	int				skillIndex;
	int				rarity;
	int				slotSize;
};

extern std::vector<Decorator*> g_decorators;

void LoadDecorators();
void FilterDecorators(const EvaluatingSkills& evSkills);
