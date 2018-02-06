#pragma once

#include "Vector.h"
#include "Matrix.h"

class Robot;

struct RobotBuilder
{
	Robot* robot;

	RobotBuilder(Robot* robot);

	void CreateBody(
		const std::string& name,
		const std::string& parentName,
		const Core::Vector3D& pos,
		const Core::Vector3D& size,
		const Core::Matrix4D& worldTx);
};
