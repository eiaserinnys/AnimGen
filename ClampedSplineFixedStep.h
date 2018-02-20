#pragma once

#include "Spline.h"

class IClampedSplineFixedStep : public ISpline {
public:
	static IClampedSplineFixedStep* Create(
		double timeStep, 
		const std::vector<Core::Vector3D>& p,
		const std::vector<Core::Vector3D>& r);
};
