#pragma once

#include <vector>
#include "Vector.h"

class ISpline {
public:
	virtual ~ISpline() = 0;

	virtual std::pair<Core::Vector3D, Core::Vector3D> At(double v) = 0;

	virtual std::pair<Core::Vector3D, Core::Vector3D> AccelerationAt(double v)
	{
		return std::make_pair(Core::Vector3D(0, 0, 0), Core::Vector3D(0, 0, 0));
	}

	virtual void SetValue(int index, int channel, double v) {}

	virtual double GetMax() = 0;
};
