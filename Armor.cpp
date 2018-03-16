#include "pch.h"
#include "Armor.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Decorator.h"

using namespace std;

vector<Set*> g_sets;

map<Armor::PartType, vector<Armor*>*> g_armors;

////////////////////////////////////////////////////////////////////////////////
bool Armor::IsRelevant(const EvaluatingSkills& evSkills) const
{
	// 셋트 스킬이 포함시킬 스킬인 경우
	int index = evSkills.GetIndex(set->skill);
	if (index >= 0) { return true; }

	// 개별 파트 스킬이 포함시킬 스킬인 경우
	for (int k = 0; k < skills.size(); ++k)
	{
		int index = evSkills.GetIndex(skills[k].first);
		if (index >= 0)
		{
			return true;
		}
	}

	for (auto& sd : evSkills.list)
	{
		if (sd.decorator == nullptr) { continue; }

		for (auto s : slots)
		{
			if (s == sd.decorator->slotSize)
			{
				return true;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void Armor::Dump(bool addNewLine) const
{
	WindowsUtility::Debug(L"%s", name.c_str());

	for (auto s : slots)
	{
		WindowsUtility::Debug(L" [%d]", s);
	}

	if (!skills.empty() || !set->skill.empty())
	{
		WindowsUtility::Debug(L" (");

		bool written = false;
		auto Write = [&](const wstring& name, int lv)
		{
			WindowsUtility::Debug(
				L"%s%s Lv.%d",
				written ? L"," : L"",
				name.c_str(),
				lv);
			written = true;
		};

		if (!set->skill.empty())
		{
			Write(set->skill, 1);
		}

		for (auto s : skills)
		{
			Write(s.first, s.second);
		}

		WindowsUtility::Debug(L")");
	}

	if (addNewLine)
	{
		WindowsUtility::Debug(L"\n");
	}
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void LoadArmors(bool dump)
{
	FILE* file;

	fopen_s(&file, "ArmorData", "r,ccs=UTF-8");

	Set* curSet = nullptr;
	int offset = 0;

	g_armors.insert(make_pair(Armor::Head, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Body, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Arm, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Waist, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Leg, new vector<Armor*>));

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

		if (offset == 0)
		{
			curSet = new Set;
			curSet->name = tokens[0];
			curSet->rarity = _wtoi(tokens[1].c_str());
			curSet->defense = _wtoi(tokens[2].c_str());
			if (tokens.size() >= 7) { curSet->skill = tokens[6].c_str(); }
			g_sets.push_back(curSet);
		}
		else if (offset < 6)
		{
			if (tokens.size() >= 3)
			{
				auto part = new Armor;
				part->type = (Armor::PartType) (offset - 1);
				part->set = curSet;
				part->name = tokens[0];

				int skillCount = _wtoi(tokens[1].c_str());
				for (int i = 0; i < skillCount; ++i)
				{
					part->skills.push_back(make_pair(
						tokens[2 + i * 2 + 0],
						_wtoi(tokens[2 + i * 2 + 1].c_str())));
				}

				int slotCount = _wtoi(tokens[2 + skillCount * 2].c_str());
				for (int i = 0; i < slotCount; ++i)
				{
					part->slots.push_back(
						_wtoi(tokens[3 + skillCount * 2 + i].c_str()));
				}

				g_armors[part->type]->push_back(part);

				if (dump)
				{
					WindowsUtility::Debug(L"\"%s\"\t", part->name.c_str());
					for (int i = 0; i < part->skills.size(); ++i)
					{
						WindowsUtility::Debug(
							L"%s Lv.%d\t",
							part->skills[i].first.c_str(),
							part->skills[i].second);
					}
					if (!part->set->skill.empty())
					{
						WindowsUtility::Debug(
							L"%s\t",
							part->set->skill.c_str());
					}
					for (int i = 0; i < part->slots.size(); ++i)
					{
						WindowsUtility::Debug(L"[%d] ", part->slots[i]);
					}
					WindowsUtility::Debug(L"\n");
				}
			}
		}

		offset = (offset + 1) % 7;
	}

	fclose(file);
}

