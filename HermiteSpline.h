#pragma once

#include <vector>
#include "Vector.h"

class IHermiteSpline {
public:
	virtual ~IHermiteSpline() = 0;

	virtual std::pair<Core::Vector3D, Core::Vector3D> At(double v) = 0;

	static IHermiteSpline* Create(
		const std::vector<Core::Vector3D>& p, 
		const std::vector<Core::Vector3D>& r);
};
