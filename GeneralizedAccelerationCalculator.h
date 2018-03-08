#pragma once

#include "SolutionVector.h"
#include "SolutionSpline.h"

#include "GeneralizedCoordinate.h"

//------------------------------------------------------------------------------
struct GeneralizedAccelerationCalculator
{
	SolutionSpline spline[7];
	GeneralizedCoordinate acc;

	GeneralizedAccelerationCalculator(
		const ISolutionVector* sv, 
		int phaseIndexAt, 
		bool dump = false);

	double BuildData(
		std::vector<std::pair<double, GeneralizedCoordinate>>& gc,
		const ISolutionVector* sv,
		int phaseIndexAt);

	void BuildSpline(
		const std::vector<std::pair<double, GeneralizedCoordinate>>& gc);

	void CalculateAcceleration(double timeToQuery);

	GeneralizedCoordinate Get();
};
