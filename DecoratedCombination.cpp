#include "pch.h"
#include "DecoratedCombination.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "Skill.h"
#include "Decorator.h"
#include "Charm.h"
#include "GeneralizedArmor.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

int instance = 0;

//------------------------------------------------------------------------------
void DecoratedCombination::Delete()
{
	refCount--;

	assert(refCount >= 0);

	if (refCount == 0)
	{
		if (derivedFrom != nullptr)
		{
			derivedFrom->Delete();
			derivedFrom = nullptr;
		}
		delete this;
	}
}

//------------------------------------------------------------------------------
DecoratedCombination::DecoratedCombination()
{
	instance++;
}

//------------------------------------------------------------------------------
DecoratedCombination* DecoratedCombination::DeriveFrom(
	const GeneralizedCombination* comb)
{
	auto ret = new DecoratedCombination;

	(*ret).ParentType::operator = (*comb);

	ret->source = comb;

	return ret;
}

//------------------------------------------------------------------------------
DecoratedCombination* DecoratedCombination::DeriveFrom(
	DecoratedCombination* from)
{
	return DeriveFrom(from, nullptr, -1, -1);
}

//------------------------------------------------------------------------------
DecoratedCombination* DecoratedCombination::DeriveFrom(
	DecoratedCombination* from,
	const Decorator* dec,
	int socket,
	int decIndex)
{
	auto ret = new DecoratedCombination;

	(*ret).ParentType::operator = (*from);

	ret->source = from->source;

	ret->derivedFrom = from;

	from->refCount++;

	if (dec != nullptr)
	{
		auto index = dec->skillIndex;

		if (ret->skills[index] + 1 <= g_skillMaxLevel[index])
		{
			ret->skills[index]++;
			ret->decorator = dec;
		}

		ret->lastDecoratorIndex = decIndex;
	}

	if (socket >= 0)
	{
		ret->slots[socket]--;	// ��·�� ������ �Һ�
		ret->lastSocket = socket;
	}

	return ret;
}

//------------------------------------------------------------------------------
DecoratedCombination::~DecoratedCombination()
{
	assert(refCount == 0);

	for (auto e : equivalents)
	{
		e->Delete();
	}
	equivalents.clear();

	instance--;
}

//------------------------------------------------------------------------------
void DecoratedCombination::CombineEquivalent(DecoratedCombination& rhs)
{
	equivalents.push_back(&rhs);
	equivalents.insert(equivalents.end(), rhs.equivalents.begin(), rhs.equivalents.end());
	rhs.equivalents.clear();
}

//------------------------------------------------------------------------------
void DecoratedCombination::Write(FILE* file) const
{
	// ù��° ���ո� ��������
	auto inst = *source->instances.begin();
	for (int i = 0; i < COUNT_OF(inst->parts); ++i)
	{
		fwprintf(file, L"%s ", (*inst->parts[i]->source.begin())->name.c_str());
	}

	if (inst->charm != nullptr)
	{
		fwprintf(file, L"%s ", inst->charm->name.c_str());
	}

	const DecoratedCombination* c = this;
	while (c != nullptr)
	{
		if (c->decorator != nullptr)
		{
			fwprintf(file, L"%s ", c->decorator->name.c_str());
		}

		c = c->derivedFrom;
	}
}

////////////////////////////////////////////////////////////////////////////////

void PopulateDecorators(
	const list<GeneralizedCombination*>& g_all, 
	list<DecoratedCombination*>& g_decAll,
	IDamageCalculator* damageCalc,
	bool dumpComparison)
{
	FILE* file;

	fopen_s(&file, "log_decorated.txt", "w,ccs=UNICODE");

	// ��Ŀ����Ƽ�� �ĺ���̼��� ���� �����
	for (auto comb : g_all)
	{
		g_decAll.push_back(DecoratedCombination::DeriveFrom(comb));
	}

	// ���� ���Ϻ��� ä������ ����Ʈ�� �����غ���
	// ���� ũ�⿡ ���ؼ� ��ȸ�ϴ� �Ϳ� ����
	bool first = true;

	for (int socket = 2; socket >= 0; )
	{
		if (first)
		{
			fwprintf(file, L"Filling socket with size %d\n", socket + 1);
			WindowsUtility::Debug(L"Filling socket with size %d\n", socket + 1);
			first = false;
		}

		list<DecoratedCombination*> next;

		// ���� �� ���Կ� ���� �� ���� ������ �ű��
		for (auto it = g_decAll.begin(); it != g_decAll.end(); )
		{
			auto cur = *it;
			if (cur->slots[socket] <= 0)
			{
				next.push_back(cur);
				it = g_decAll.erase(it);
			}
			else
			{
				it++;
			}
		}

		fwprintf(file, L"%d combinations moved\n", (int)next.size());
		WindowsUtility::Debug(L"%d combinations moved\n", next.size());

		int evaluated = 0;
		int rejected[] = { 0, 0, 0, };
		for (auto it = g_decAll.begin(); it != g_decAll.end(); ++it)
		{
			auto cur = *it;
			assert(cur->slots[socket] > 0);

			// ��� ���� ������ �� ���� ������ �� ������
			// �̷��� ������ ����ָ� ����鼭 ������� ����� ��
			// ���� n���� ������ �� �ִ�
			vector<int> decorators;

			auto temp = DecoratedCombination::DeriveFrom(cur);

			function<void(int)> BuildCombination;
			BuildCombination = [&](int iterateFrom)
			{
				for (int d = iterateFrom; d < g_decorators.size(); ++d)
				{
					auto dec = g_decorators[d];

					if (dec->slotSize != socket + 1) { continue; }

					bool notMaxed =
						temp->skills[dec->skillIndex] + 1 <=
						g_skillMaxLevel[dec->skillIndex];

					if (!notMaxed) { continue; }

					temp->skills[dec->skillIndex]++;
					temp->slots[socket]--;
					decorators.push_back(d);

					if (temp->slots[socket] > 0)
					{
						BuildCombination(d);
					}
					else
					{
						// ���� �� ������ ����� �־��
						++evaluated;
						if (evaluated % 10000 == 0)
						{
							WindowsUtility::Debug(
								L"\t%d (evaluated=%d)\n",
								next.size(),
								evaluated);
						}

						auto newComb = DecoratedCombination::DeriveFrom(cur);

						for (int di : decorators)
						{
							auto si = g_decorators[di]->skillIndex;
							newComb->skills[si]++;
							newComb->slots[socket]--;

							assert(newComb->skills[si] <= g_skillMaxLevel[si]);
						}

						assert(newComb->slots[socket] >= 0);

						AddIfNotWorse(next, newComb, false, nullptr, dumpComparison);
					}

					temp->skills[dec->skillIndex]--;
					temp->slots[socket]++;
					decorators.pop_back();
				}
			};

			BuildCombination(0);

			temp->Delete();
		}

		socket--;
		first = true;

		for (auto inst : g_decAll) { inst->Delete(); }

		g_decAll.swap(next);

		fwprintf(
			file,
			L"\t%d (evaluated=%d)\n",
			(int)g_decAll.size(),
			evaluated);

		WindowsUtility::Debug(
			L"\t%d (evaluated=%d)\n",
			g_decAll.size(),
			evaluated);
	}

	fclose(file);
}

