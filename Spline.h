#pragma once

#include <vector>
#include "Vector.h"

class ISpline {
public:
	virtual ~ISpline() = 0;

	virtual std::pair<Core::Vector3D, Core::Vector3D> At(double v) = 0;

	virtual double GetMax() = 0;
};
