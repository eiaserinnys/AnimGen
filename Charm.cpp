#include "pch.h"
#include "Charm.h"

#include <Utility.h>
#include "Skill.h"

using namespace std;

vector<Charm*> g_allCharms;
vector<Charm*> g_charms;

//------------------------------------------------------------------------------
void LoadCharms()
{
	FILE* file;

	fopen_s(&file, "CharmData", "r,ccs=UTF-8");

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

		if (tokens.size() >= 3)
		{
			auto dec = new Charm;
			dec->name = tokens[0];

			for (int i = 1; i < tokens.size() - 1; i++)
			{
				dec->skills.push_back(make_pair(tokens[i], -1));
			}

			dec->skillLevel = _wtoi(tokens.rbegin()->c_str());

			g_allCharms.push_back(dec);
		}
	}

	fclose(file);
}

//------------------------------------------------------------------------------
void FilterCharms(const EvaluatingSkills& evSkills)
{
	g_charms.clear();

	for (auto c : g_allCharms)
	{
		bool toAdd = false;

		for (int i = 0; i < c->skills.size(); ++i)
		{
			int index = evSkills.GetIndex(c->skills[i].first);
			if (index >= 0)
			{
				c->skills[i].second = index;
				toAdd = true;
			}
		}

		if (toAdd)
		{
			g_charms.push_back(c);
		}
	}
}
