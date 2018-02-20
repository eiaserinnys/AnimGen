#pragma once

#include "Spline.h"

class IHermiteSpline : public ISpline {
public:
	static IHermiteSpline* Create(
		const std::vector<Core::Vector3D>& p, 
		const std::vector<Core::Vector3D>& r);
};
