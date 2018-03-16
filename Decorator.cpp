#include "pch.h"
#include "Decorator.h"

#include <Utility.h>
#include "Skill.h"

using namespace std;

vector<Decorator*> g_allDecorators;
vector<Decorator*> g_decorators;

//------------------------------------------------------------------------------
void LoadDecorators()
{
	FILE* file;

	fopen_s(&file, "DecorationData", "r,ccs=UTF-8");

	wchar_t buffer[1024];
	while (fgetws(&buffer[0], 1024, file) != nullptr)
	{
		// 먼저 뉴라인 문자를 제거한다
		wstring str = buffer;
		size_t pos;
		if ((pos = str.find(L'\n')) != wstring::npos)
		{
			str = str.substr(0, pos);
		}

		// 쉼표로 자른다
		auto tokens = Utility::Tokenize(str);

		if (tokens.size() >= 4)
		{
			auto dec = new Decorator;
			dec->name = tokens[3];
			dec->skill = tokens[0];
			dec->skillIndex = -1;
			dec->rarity = _wtoi(tokens[2].c_str());
			dec->slotSize = _wtoi(tokens[1].c_str());

			g_allDecorators.push_back(dec);
		}
	}

	fclose(file);
}

//------------------------------------------------------------------------------
void FilterDecorators(const EvaluatingSkills& evSkills)
{
	g_decorators.clear();

	for (auto dec : g_allDecorators)
	{
		int index = evSkills.GetIndex(dec->skill);
		if (index >= 0)
		{
			dec->skillIndex = index;
			g_decorators.push_back(dec);
		}
	}
}
