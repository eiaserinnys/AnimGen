#pragma once

#include <string>
#include <vector>

struct Charm
{
	std::wstring name;
	std::wstring skill;
	int skillIndex;
	int skillLevel;
};

extern std::vector<Charm*> g_charms;

