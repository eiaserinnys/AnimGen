#pragma once

class ISolver {
public:
	virtual ~ISolver() {}

	struct Result
	{
		enum Value
		{
			Solved,
			StepAccepted,
			StepRejected,
			Unsolvable,
		};
	};

	virtual void Begin() = 0;

	virtual Result::Value SolveStep() = 0;

	virtual void End() = 0;

	static ISolver* Create();
};