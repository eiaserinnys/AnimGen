#pragma once

#include "Robot.h"

class Robot;

struct RobotCoordinate
{
	IRobot::GeneralCoordinate ToGeneralCoordinate(Robot* robot);

	void FromGeneralCoordinate(
		Robot* robot,
		const IRobot::GeneralCoordinate& coord);
};