#pragma once

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

	~CombinationBase();

	void Combine(const CombinationBase& rhs);

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

	void DumpSimple() const;
};
