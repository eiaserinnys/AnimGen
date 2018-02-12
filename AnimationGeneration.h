#pragma once

class IRobot;
class LineBuffer;

class IAnimationGeneration {
public:
	~IAnimationGeneration() {}

	virtual void UpdateSpline() = 0;

	virtual void Enqueue(LineBuffer* buffer) = 0;

	static IAnimationGeneration* Create(IRobot* robot);
};