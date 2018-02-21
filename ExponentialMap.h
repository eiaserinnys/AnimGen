#pragma once

#include "Vector.h"
#include "Matrix.h"

struct ExponentialMap 
{
	static Core::Vector3D Negate(const Core::Vector3D& exp);
	static Core::Vector3D GetNearRotation(const Core::Vector3D& pivot, const Core::Vector3D& toEval);
	static void MakeNearRotation(const Core::Vector3D& pivot, Core::Vector3D& toEval);

	static Core::Vector3D FromQuaternion(const Core::Vector4D& quat);
	static Core::Vector4D ToQuaternion(const Core::Vector3D& expMap, bool reg = false);

	static Core::Vector3D FromMatrix(const Core::Matrix4D& mat);
	static Core::Matrix4D ToMatrix(const Core::Vector3D& expMap);
};