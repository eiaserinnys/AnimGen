#pragma once

#include "SolutionVector.h"
#include "SolutionSpline.h"
#include "RobotCoordinate.h"

//------------------------------------------------------------------------------
struct GeneralizedAccelerationCalculator
{
	SolutionSpline spline[7];
	GeneralCoordinate acc;

	GeneralizedAccelerationCalculator(ISolutionVector* sv, int phaseIndexAt, bool dump);

	double BuildData(
		std::vector<std::pair<double, GeneralCoordinate>>& gc,
		ISolutionVector* sv,
		int phaseIndexAt);

	void BuildSpline(
		const std::vector<std::pair<double, GeneralCoordinate>>& gc);

	void CalculateAcceleration(double timeToQuery);

	GeneralCoordinate Get();
};
