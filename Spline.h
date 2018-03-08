#pragma once

#include <vector>
#include "PositionRotation.h"

class ISpline {
public:
	virtual ~ISpline() = 0;

	virtual PositionRotation At(double v) = 0;

	virtual PositionRotation AccelerationAt(double v)
	{
		return PositionRotation::Zero();
	}

	virtual void SetValue(int index, int channel, double v) {}

	virtual double GetMax() = 0;
};
