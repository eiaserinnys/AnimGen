#pragma once

#include <WindowsUtility.h>

//------------------------------------------------------------------------------
struct CombinationBase
{
	int* skills = nullptr;
	int skillCount = 0;

	int slots[3];

	CombinationBase() = default;

	CombinationBase(const CombinationBase& rhs);

	CombinationBase& operator = (const CombinationBase& rhs);

	CombinationBase(int skillCount);

	virtual ~CombinationBase();
	
	virtual void CombineEquivalent(CombinationBase& rhs) {}

	virtual void Delete() { delete this; }

	void Combine(const CombinationBase& rhs);

	bool IsTriviallyWorse(const CombinationBase& rhs) const;

	bool IsTriviallyEquivalent(const CombinationBase& rhs) const;

	int SlotCount() const;

	enum ComparisonResult
	{
		Better,
		Undetermined,

		NotWorse,

		Worse,
		Equal,
	};

	static ComparisonResult CompareStrict2(
		const CombinationBase& lhs,
		const CombinationBase& rhs);

	static ComparisonResult CompareStrict2(
		const CombinationBase& lhs,
		const CombinationBase& rhs,
		int skillToAddToLhs,
		int lhsSocketToUse);

	static ComparisonResult Compare(
		const CombinationBase& lhs,
		const CombinationBase& rhs);

	static ComparisonResult CompareStrict(
		const CombinationBase& lhs,
		const CombinationBase& rhs);

	static ComparisonResult Compare(
		const CombinationBase& lhs,
		const CombinationBase& rhs,
		int skillToAddToLhs,
		int lhsSocketToUse);

	static ComparisonResult CompareStrict(
		const CombinationBase& lhs,
		const CombinationBase& rhs,
		int skillToAddToLhs,
		int lhsSocketToUse);

	ComparisonResult Compare(const CombinationBase& rhs) const;

	bool IsWorseThanOrEqualTo(const CombinationBase& rhs) const;

	void Dump() const;

	void Dump(FILE* file) const;

	void DumpSimple() const;

	std::wstring DumpToString() const;
};

//------------------------------------------------------------------------------
template <typename CombinationType>
void AddIfNotWorse(
	std::list<CombinationType*>& container,
	CombinationType* newPart,
	bool deleteEquivalent, 
	FILE* file, 
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
				if (!newPart->IsTriviallyEquivalent(*prev))
				{
					fwprintf(
						file,
						L"\t%s == %s\n",
						newPart->DumpToString().c_str(),
						prev->DumpToString().c_str());
				}
			}

			prev->CombineEquivalent(*newPart);

			if (deleteEquivalent)
			{
				newPart->Delete();
			}

			newPart = nullptr;
			break;
		}
		else if (compare == GeneralizedArmor::Better)
		{
			if (dump)
			{
				if (!prev->IsTriviallyWorse(*newPart))
				{
					fwprintf(
						file,
						L"\t%s < %s\n",
						prev->DumpToString().c_str(),
						newPart->DumpToString().c_str());
				}
			}

			prev->Delete();
			it = container.erase(it);
		}
		else if (compare == GeneralizedArmor::Worse)
		{
			if (dump)
			{
				if (!newPart->IsTriviallyWorse(*prev))
				{
					fwprintf(
						file,
						L"\t%s < %s\n",
						newPart->DumpToString().c_str(),
						prev->DumpToString().c_str());
				}
			}

			newPart->Delete();
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

		//if (dump)
		//{
		//	WindowsUtility::Debug(L"\t");
		//	newPart->DumpSimple();
		//	WindowsUtility::Debug(L" is added.\n");
		//}
	}
}

//------------------------------------------------------------------------------
template <typename CombinationType>
int RejectWorseCombinations(
	std::list<CombinationType*>& next,
	bool dump)
{
	// 생성된 페어를 평가한다
	int rejected = 0;
	for (auto it = next.begin(); it != next.end(); )
	{
		auto toEvaluate = *it;

		bool bad = false;

		auto jt = it;
		jt++;

		for (; jt != next.end();)
		{
			auto target = *jt;

			auto result3 = GeneralizedCombination::CompareStrict2(*toEvaluate, *target);

			if (result3 == GeneralizedCombination::Equal)
			{
				// 우측을 좌측에 더하자
				toEvaluate->CombineEquivalent(*target);

				if (dump)
				{
					WindowsUtility::Debug(L"Dropping '");
					target->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"'. (Same)\n");
				}

				++rejected;
				jt = next.erase(jt);
			}
			else if (result3 == GeneralizedCombination::Worse)
			{
				if (dump)
				{
					WindowsUtility::Debug(L"Dropping '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					target->DumpSimple();
					WindowsUtility::Debug(L"'. (Worse)\n");
				}

				bad = true;
				break;
			}
			else if (result3 == GeneralizedCombination::Better)
			{
				if (dump)
				{
					WindowsUtility::Debug(L"Dropping '");
					target->DumpSimple();
					WindowsUtility::Debug(L"' for '");
					toEvaluate->DumpSimple();
					WindowsUtility::Debug(L"'. (Worse)\n");
				}

				++rejected;
				target->Delete();
				jt = next.erase(jt);
			}
			else // Undetermined
			{
				jt++;
			}
		}

		if (bad)
		{
			++rejected;
			toEvaluate->Delete();
			it = next.erase(it);
		}
		else
		{
			it++;
		}
	}

	return rejected;
}