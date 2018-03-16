#pragma once

#include <string>
#include <vector>

#include "Skill.h"

struct Charm
{
	std::wstring name;

	std::vector<std::pair<std::wstring, int>> skills;

	int skillLevel;
};

extern std::vector<Charm*> g_charms;

void LoadCharms();
void FilterCharms(const EvaluatingSkills& evSkills);