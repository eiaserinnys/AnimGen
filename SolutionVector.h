#pragma once

#include <vector>

#include "RobotCoordinate.h"

struct SolutionVector
{
	std::vector<std::pair<double, SolutionCoordinate>> coords;

	static SolutionVector BuildTest(const SolutionCoordinate& init);
};