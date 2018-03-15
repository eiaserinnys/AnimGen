#include "pch.h"
#include "GeneralizedArmor.h"

#include "Skill.h"

#include <Utility.h>
#include <WindowsUtility.h>

using namespace std;

//------------------------------------------------------------------------------
GeneralizedArmor::GeneralizedArmor(const Armor* part)
	: ParentType(COUNT_OF(g_skills))
{
	// 셋트 스킬이 포함시킬 스킬인 경우
	int index = GetSkillIndex(part->set->skill);
	if (index >= 0) { skills[index]++; }

	// 개별 파트 스킬이 포함시킬 스킬인 경우
	for (int k = 0; k < part->skills.size(); ++k)
	{
		int index = GetSkillIndex(part->skills[k].first);
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
void GeneralizedArmor::CombineSource(const GeneralizedArmor& rhs)
{
	source.insert(source.end(), rhs.source.begin(), rhs.source.end());
}

//------------------------------------------------------------------------------
void GeneralizedArmor::Dump() const
{
	DumpSimple();

	WindowsUtility::Debug(L"\n");

	for (auto it = source.begin(); it != source.end(); ++it)
	{
		WindowsUtility::Debug(L"\t%s\n", (*it)->name.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void AddIfNotWorse(
	vector<GeneralizedArmor*>& container,
	GeneralizedArmor* newPart,
	bool dump)
{
	for (auto it = container.begin(); it != container.end(); )
	{
		auto prev = *it;

		auto compare = CombinationBase::CompareStrict2(*newPart, *prev);

		if (compare == GeneralizedArmor::Equal)
		{
			if (dump)
			{ 
				WindowsUtility::Debug(L"\t");
				newPart->DumpSimple();
				WindowsUtility::Debug(L" is equivalent to ");
				prev->DumpSimple();
				WindowsUtility::Debug(L"\n");
			}

			prev->CombineSource(*newPart);

			delete newPart;
			newPart = nullptr;

			break;
		}
		else if (compare == GeneralizedArmor::Better)
		{
			if (dump)
			{
				WindowsUtility::Debug(L"\t");
				newPart->DumpSimple();
				WindowsUtility::Debug(L" is rejecting ");
				prev->DumpSimple();
				WindowsUtility::Debug(L"\n");
			}

			delete prev;
			it = container.erase(it);
		}
		else if (compare == GeneralizedArmor::Worse)
		{
			if (dump)
			{
				WindowsUtility::Debug(L"\t");
				newPart->DumpSimple();
				WindowsUtility::Debug(L" is rejected by ");
				prev->DumpSimple();
				WindowsUtility::Debug(L"\n");
			}

			delete newPart;
			newPart = nullptr;
			break;
		}
		else
		{
			++it;
		}
	}

	if (newPart != nullptr)
	{
		container.push_back(newPart);

		if (dump)
		{
			WindowsUtility::Debug(L"\t");
			newPart->DumpSimple();
			WindowsUtility::Debug(L" is added.\n");
		}
	}
}

//------------------------------------------------------------------------------
void FilterArmors(
	map<Armor::PartType, vector<GeneralizedArmor*>*>& g_generalized,
	bool dump)
{
	g_generalized.insert(make_pair(Armor::Head, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Body, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Arm, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Waist, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Leg, new vector<GeneralizedArmor*>));
		
	int total = 1;

	for (int i = 0; i < Armor::Count; ++i)
	{
		auto& container = *g_generalized[(Armor::PartType) i];

		if (dump)
		{
			WindowsUtility::Debug(L"----------------------------------------\n");
		}

		auto parts = g_armors[(Armor::PartType) i];
		for (int j = 0; j < parts->size(); ++j)
		{
			auto part = (*parts)[j];

			// 관심 있는 스킬이 있거나 슬롯이 있거나 해야 한다
			if (!part->IsRelevant()) 
			{ 
				//WindowsUtility::Debug(L"\tDiscarded\n");
				continue; 
			}
			
			if (dump)
			{
				part->Dump(true);
			}

			// 계산을 위해서 베이스를 만든다
			GeneralizedArmor base(part);

			// 여기서 슬롯에 끼울 수 있는 장식주 크기별로 조합을 새로 만든다
			int subtract[] = { 0, 0, 0, };
			int add[] = { 0, 0, 0, };

			auto TryToAdd = [&]()
			{
				auto newPart = new GeneralizedArmor(part);

				for (int i = 0; i < 3; ++i)
				{
					newPart->slots[i] += add[i] - subtract[i];
				}

				AddIfNotWorse(container, newPart, dump);
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

		WindowsUtility::Debug(L"----------------------------------------\n");

		int validParts = g_generalized[(Armor::PartType) i]->size();
		WindowsUtility::Debug(L"%d effective parts found\n", validParts);

		for (auto general : *g_generalized[(Armor::PartType) i])
		{
			general->Dump();
		}

		total *= validParts;
	}

	WindowsUtility::Debug(L"Total Combination = %d\n", total);
}

