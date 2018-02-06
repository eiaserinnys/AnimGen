#pragma once

#include "Robot.h"

class Robot;

struct RobotCoordinate
{
	IRobot::GeneralCoordinate CalculateGeneralCoordinate(Robot* robot);
};