#pragma once

#include <vector>

#include "RobotCoordinate.h"

struct SolutionVector
{
	std::vector<SolutionCoordinate> coords;

	static SolutionVector BuildTest(const SolutionCoordinate& init);
};