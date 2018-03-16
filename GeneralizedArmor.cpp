#include "pch.h"
#include "GeneralizedArmor.h"

#include "Skill.h"

#include <Utility.h>
#include <WindowsUtility.h>

using namespace std;

//------------------------------------------------------------------------------
GeneralizedArmor::GeneralizedArmor(
	const EvaluatingSkills& evSkills, const Armor* part)
	: ParentType(evSkills.list.size())
{
	// 셋트 스킬이 포함시킬 스킬인 경우
	int index = evSkills.GetIndex(part->set->skill);
	if (index >= 0) { skills[index]++; }

	// 개별 파트 스킬이 포함시킬 스킬인 경우
	for (int k = 0; k < part->skills.size(); ++k)
	{
		int index = evSkills.GetIndex(part->skills[k].first);
		if (index >= 0)
		{
			skills[index] += part->skills[k].second;
		}
	}

	// 슬롯을 적용한다
	for (int k = 0; k < part->slots.size(); ++k)
	{
		slots[part->slots[k] - 1]++;
	}

	source.push_back(part);
}

//------------------------------------------------------------------------------
GeneralizedArmor::GeneralizedArmor(const GeneralizedArmor& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
GeneralizedArmor& GeneralizedArmor::operator = (const GeneralizedArmor& rhs)
{
	ParentType::operator = (rhs);

	source = rhs.source;

	return *this;
}

//------------------------------------------------------------------------------
void GeneralizedArmor::CombineEquivalent(const GeneralizedArmor& rhs)
{
	source.insert(source.end(), rhs.source.begin(), rhs.source.end());
}

//------------------------------------------------------------------------------
void GeneralizedArmor::Dump(const EvaluatingSkills& evSkills) const
{
	DumpSimple(evSkills);

	WindowsUtility::Debug(L"\n");

	for (auto it = source.begin(); it != source.end(); ++it)
	{
		WindowsUtility::Debug(L"\t%s\n", (*it)->name.c_str());
	}
}

//------------------------------------------------------------------------------
void GeneralizedArmor::Dump(const EvaluatingSkills& evSkills, FILE* file) const
{
	ParentType::Dump(evSkills, file);

	for (auto it = source.begin(); it != source.end(); ++it)
	{
		fwprintf(file, L"\t%s\n", (*it)->name.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void FilterArmors(
	const EvaluatingSkills& evSkills,
	map<Armor::PartType, list<GeneralizedArmor*>*>& g_generalized,
	bool dumpList,
	bool dumpComparison)
{
	FILE* file;
	fopen_s(&file, "log_armor.txt", "w,ccs=UNICODE");

	g_generalized.insert(make_pair(Armor::Head, new list<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Body, new list<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Arm, new list<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Waist, new list<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Leg, new list<GeneralizedArmor*>));
		
	int total = 1;

	for (int i = 0; i < Armor::Count; ++i)
	{
		auto& container = *g_generalized[(Armor::PartType) i];

		if (dumpComparison)
		{
			fwprintf(file, L"----------------------------------------\n");
		}

		auto parts = g_armors[(Armor::PartType) i];
		for (int j = 0; j < parts->size(); ++j)
		{
			auto part = (*parts)[j];

			// 관심 있는 스킬이 있거나 슬롯이 있거나 해야 한다
			if (!part->IsRelevant(evSkills)) 
			{ 
				//WindowsUtility::Debug(L"\tDiscarded\n");
				continue; 
			}
			
			if (dumpComparison)
			{
				//part->Dump(true);
			}

			// 계산을 위해서 베이스를 만든다
			GeneralizedArmor base(evSkills, part);

			// 여기서 슬롯에 끼울 수 있는 장식주 크기별로 조합을 새로 만든다
			int subtract[] = { 0, 0, 0, };
			int add[] = { 0, 0, 0, };

			auto TryToAdd = [&]()
			{
				auto newPart = new GeneralizedArmor(evSkills, part);

				for (int i = 0; i < 3; ++i)
				{
					newPart->slots[i] += add[i] - subtract[i];
				}

				AddIfNotWorse(evSkills, container, newPart, false, file, true);
			};

			function<void(int)> Permutate;
			Permutate = [&](int socket)
			{
				if (socket + 1 < 3 && base.slots[socket + 1] > 0)
				{
					Permutate(socket + 1);
				}

				if (base.slots[socket] - subtract[socket] > 0)
				{
					subtract[socket]++;

					for (int i = socket - 1; i >= 0; --i)
					{
						add[i]++;

						// 슬롯을 바꿀 때마다 추가해보자
						TryToAdd();

						Permutate(socket);

						add[i]--;
					}

					subtract[socket]--;
				}
			};

			// 아무 조작도 안 한 상태도 추가
			TryToAdd();

			Permutate(1);
		}

		fwprintf(file, L"----------------------------------------\n");

		int validParts = g_generalized[(Armor::PartType) i]->size();

		fwprintf(file, L"%d effective parts found\n", validParts);
		WindowsUtility::Debug(L"%d effective parts found\n", validParts);

		if (dumpList)
		{
			for (auto general : *g_generalized[(Armor::PartType) i])
			{
				general->Dump(evSkills, file);
			}
		}

		total *= validParts;
	}

	fwprintf(file, L"Total Possible Combination = %d\n", total);
	WindowsUtility::Debug(L"Total Possible Combination = %d\n", total);

	fclose(file);
}

