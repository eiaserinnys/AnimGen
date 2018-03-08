#pragma once

#include "SolutionVector.h"
#include "SolutionSpline.h"

#include "GeneralizedCoordinate.h"

//------------------------------------------------------------------------------
struct GeneralizedAccelerationCalculator
{
	SolutionSpline spline[7];
	GeneralCoordinate acc;

	GeneralizedAccelerationCalculator(
		const ISolutionVector* sv, 
		int phaseIndexAt, 
		bool dump = false);

	double BuildData(
		std::vector<std::pair<double, GeneralCoordinate>>& gc,
		const ISolutionVector* sv,
		int phaseIndexAt);

	void BuildSpline(
		const std::vector<std::pair<double, GeneralCoordinate>>& gc);

	void CalculateAcceleration(double timeToQuery);

	GeneralCoordinate Get();
};
