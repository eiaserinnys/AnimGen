#pragma once

struct SolutionCoordinate;

class ISolver {
public:
	virtual ~ISolver() = 0;

	virtual void Begin() = 0;

	struct Result
	{
		enum Value
		{
			Unsolvable = -1,
			Solved = 0,
			StepAccepted = 1,
			StepRejected = 2,
		};
	};

	virtual Result::Value SolveStep() = 0;

	virtual void End() = 0;

	static ISolver* Create(
		const SolutionCoordinate& start,
		const SolutionCoordinate& dest,
		int phases);
};