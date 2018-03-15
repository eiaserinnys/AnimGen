#include "pch.h"
#include "Charm.h"

#include <Utility.h>
#include "Skill.h"

using namespace std;

vector<Charm*> g_charms;

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
				int index = GetSkillIndex(tokens[i]);
				if (index >= 0)
				{
					dec->skills.push_back(make_pair(tokens[i], index));
				}
			}

			dec->skillLevel = _wtoi(tokens.rbegin()->c_str());

			if (!dec->skills.empty())
			{
				g_charms.push_back(dec);
			}
			else
			{
				delete dec;
			}

			//WindowsUtility::Debug(L"%s %s R%d [%d]\n", dec->name.c_str(), dec->skill.c_str(), dec->rarity, dec->slotSize);
		}
	}

	fclose(file);
}
