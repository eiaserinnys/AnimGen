#pragma once

#include <string>

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
