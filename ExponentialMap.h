#pragma once

#include "Vector.h"
#include "Matrix.h"

struct ExponentialMap 
{
	static Core::Vector3D FromQuaternion(const Core::Vector4D& quat);
	static Core::Vector4D ToQuaternion(const Core::Vector3D& expMap);
	static Core::Matrix4D ToMatrix(const Core::Vector3D& expMap);
};