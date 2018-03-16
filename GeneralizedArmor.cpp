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
	// ��Ʈ ��ų�� ���Խ�ų ��ų�� ���
	int index = evSkills.GetIndex(part->set->skill);
	if (index >= 0) { skills[index]++; }

	// ���� ��Ʈ ��ų�� ���Խ�ų ��ų�� ���
	for (int k = 0; k < part->skills.size(); ++k)
	{
		int index = evSkills.GetIndex(part->skills[k].first);
		if (index >= 0)
		{
			skills[index] += part->skills[k].second;
		}
	}

	// ������ �����Ѵ�
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

			// ���� �ִ� ��ų�� �ְų� ������ �ְų� �ؾ� �Ѵ�
			if (!part->IsRelevant(evSkills)) 
			{ 
				//WindowsUtility::Debug(L"\tDiscarded\n");
				continue; 
			}
			
			if (dumpComparison)
			{
				//part->Dump(true);
			}

			// ����� ���ؼ� ���̽��� �����
			GeneralizedArmor base(evSkills, part);

			// ���⼭ ���Կ� ���� �� �ִ� ����� ũ�⺰�� ������ ���� �����
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

						// ������ �ٲ� ������ �߰��غ���
						TryToAdd();

						Permutate(socket);

						add[i]--;
					}

					subtract[socket]--;
				}
			};

			// �ƹ� ���۵� �� �� ���µ� �߰�
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

