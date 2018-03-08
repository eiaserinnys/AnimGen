#pragma once

#include "Vector.h"

struct PositionRotation
{
	Core::Vector3D position;
	Core::Vector3D rotation;

	double& At(int i);
	const double& At(int i) const;

	static PositionRotation Zero();
};