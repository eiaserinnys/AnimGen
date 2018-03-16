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
	auto ret = new DecoratedCombination;

	(*ret).ParentType::operator = (*from);

	ret->source = from->source;

	ret->derivedFrom = from;

	from->refCount++;

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
		for (auto dec : c->decorator)
		{
			fwprintf(file, L"%s ", dec->name.c_str());
		}

		c = c->derivedFrom;
	}
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void MoveFixedCombinations(
	list<pair<double, DecoratedCombination*>>& next,
	list<pair<double, DecoratedCombination*>>& prev,
	int socket)
{
	for (auto it = prev.begin(); it != prev.end(); )
	{
		auto cur = it->second;
		if (cur->slots[socket] <= 0)
		{
			next.push_back(make_pair(0, cur));
			it = prev.erase(it);
		}
		else
		{
			it++;
		}
	}
}

//------------------------------------------------------------------------------
void AddByDamage(
	list<pair<double, DecoratedCombination*>>& next, 
	DecoratedCombination* comb, 
	IDamageCalculator* damageCalc)
{
	const int maxCount = 150;

	auto damage = damageCalc->Do(comb);

	bool canBeAdded =
		damage > 0 && (
			next.size() < maxCount ||
			damage >= next.rbegin()->first);

	if (canBeAdded)
	{
		for (auto prev : next)
		{
			// �ϴ� ������ ���� ���������� Ȯ���Ѵ�
			if (prev.second->IsTriviallyEquivalent(*comb))
			{
				prev.second->CombineEquivalent(*comb);
				comb = nullptr;
				break;
			}
		}

		if (comb != nullptr)
		{
			// ���� ����ũ�ϱ� �ϴ�, ���ĵ� ���·� �߰�����
			for (auto it = next.begin(); it != next.end(); ++it)
			{
				if (it->first < damage)
				{
					next.insert(it, make_pair(damage, comb));

					if (next.size() > maxCount)
					{
						next.rbegin()->second->Delete();
						next.pop_back();
					}

					return;
				}
			}

			next.push_back(make_pair(damage, comb));

			if (next.size() > maxCount)
			{
				next.rbegin()->second->Delete();
				next.pop_back();
			}
		}
	}
	else
	{
		// ���� ����Ʈ�� ����ġ���� ����� ���� ������ ������
		comb->Delete();
	}
}

//------------------------------------------------------------------------------
void AddFixedCombinationsByDamage(
	list<pair<double, DecoratedCombination*>>& next,
	list<pair<double, DecoratedCombination*>>& prev,
	IDamageCalculator* damageCalc)
{
	for (auto it = prev.begin(); it != prev.end(); )
	{
		auto cur = it->second;
		if (cur->slots[0] <= 0)
		{
			AddByDamage(next, cur, damageCalc);
			it = prev.erase(it);
		}
		else
		{
			it++;
		}
	}
}

//------------------------------------------------------------------------------
void PopulateDecorators(
	const EvaluatingSkills& evSkills,
	const list<GeneralizedCombination*>& g_all, 
	list<DecoratedCombination*>& g_decAll_,
	IDamageCalculator* damageCalc,
	bool dumpComparison)
{
	FILE* file;

	fopen_s(&file, "log_decorated.txt", "w,ccs=UNICODE");

	list<pair<double, DecoratedCombination*>> intm;

	// ��Ŀ����Ƽ�� �ĺ���̼��� ���� �����
	for (auto comb : g_all)
	{
		intm.push_back(make_pair(0, DecoratedCombination::DeriveFrom(comb)));
	}

	// ���� ���Ϻ��� ä������ ����Ʈ�� �����غ���
	// ���� ũ�⿡ ���ؼ� ��ȸ�ϴ� �Ϳ� ����
	for (int socket = 2; socket >= 0; socket--)
	{
		fwprintf(file, L"Filling socket with size %d\n", socket + 1);
		WindowsUtility::Debug(L"Filling socket with size %d\n", socket + 1);

		list<pair<double, DecoratedCombination*>> next;

		// ���� �� ���Կ� ���� �� ���� ������ �ű��
		if (socket == 0 && damageCalc != nullptr)
		{
			AddFixedCombinationsByDamage(next, intm, damageCalc);
		}
		else
		{
			MoveFixedCombinations(next, intm, socket);
		}

		fwprintf(file, L"%d combinations moved\n", (int)next.size());
		WindowsUtility::Debug(L"%d combinations moved\n", next.size());

		int evaluated = 0;
		int rejected[] = { 0, 0, 0, };

		auto Report = [&](bool toFile)
		{
			WindowsUtility::Debug(L"\t%d (evaluated=%d)\n", next.size(), evaluated);
			fwprintf(file, L"\t%d (evaluated=%d)\n", (int) next.size(), evaluated);
		};

		for (auto it = intm.begin(); it != intm.end(); ++it)
		{
			auto cur = it->second;
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

					bool maxedAlready =
						temp->skills[dec->skillIndex] + 1 >
						evSkills.list[dec->skillIndex].maxLevel;
					if (maxedAlready) { continue; }

					temp->skills[dec->skillIndex]++;
					temp->slots[socket]--;
					decorators.push_back(d);

					// ���� �� �ڸ��� �������� �� ��������
					if (temp->slots[socket] > 0)
					{
						BuildCombination(d);
					}
					else
					{
						// ���� �� ������ ����� �־��
						++evaluated;
						if (evaluated % 10000 == 0) { Report(false); }

						auto newComb = DecoratedCombination::DeriveFrom(cur);

						for (int di : decorators)
						{
							auto si = g_decorators[di]->skillIndex;
							newComb->skills[si]++;
							newComb->slots[socket]--;
							newComb->decorator.push_back(g_decorators[di]);

							assert(newComb->skills[si] <= evSkills.list[si].maxLevel);
						}

						assert(newComb->slots[socket] >= 0);

						if (socket == 0 && damageCalc != nullptr)
						{
							AddByDamage(next, newComb, damageCalc);
						}
						else
						{
							AddIfNotWorse(evSkills, next, newComb, false, nullptr, dumpComparison);
						}
					}

					temp->skills[dec->skillIndex]--;
					temp->slots[socket]++;
					decorators.pop_back();
				}
			};

			BuildCombination(0);

			temp->Delete();
		}

		Report(true);

		for (auto inst : intm) { inst.second->Delete(); }
		intm.swap(next);
	}

	for (auto dc : intm)
	{
		g_decAll_.push_back(dc.second);
	}

	fclose(file);
}

