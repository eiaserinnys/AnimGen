#pragma once

#include "Vector.h"

class Robot;

struct RobotIK
{
	RobotIK(Robot* robot);

	void RobotIK::SetFootPosition(bool left, const Core::Vector3D& pos_);

	void RobotIK::SetFootTransform(
		bool left, 
		const Core::Vector3D& pos_,
		const Core::Vector3D& rot_);

	static Core::Vector3D GetFootDirection(const Core::Vector3D& legDir_, bool left);

	Robot* robot;
};