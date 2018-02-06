#pragma once

#include "Vector.h"

struct ExponentialMap 
{
	static Core::Vector3D FromQuaternion(const Core::Vector4D& quat);
	static Core::Vector4D ToQuaternion(const Core::Vector3D& expMap);
};