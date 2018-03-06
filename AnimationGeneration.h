#pragma once

#include "Solver.h"

class IRobot;
class LineBuffer;

class IAnimationGeneration {
public:
	~IAnimationGeneration() {}

	virtual void Begin() = 0;

	virtual ISolver::Result::Value Step() = 0;

	virtual void End() = 0;

	virtual void UpdateSpline() = 0;

	virtual void Enqueue(LineBuffer* buffer) = 0;

	static IAnimationGeneration* Create(IRobot* robot);
};