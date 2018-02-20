#pragma once

#include "Spline.h"

class IClampedSpline : public ISpline {
public:
	static IClampedSpline* Create(
		const std::vector<double>& t, 
		const std::vector<Core::Vector3D>& p,
		const std::vector<Core::Vector3D>& r);
};
