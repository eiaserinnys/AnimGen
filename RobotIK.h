#pragma once

#include "Vector.h"

class Robot;

struct RobotIK
{
	RobotIK(Robot* robot);

	void RobotIK::SetFootPosition(bool left, const Core::Vector3D& pos_);

	static Core::Vector3D GetFootDirection(const Core::Vector3D& legDir_);

	Robot* robot;
};