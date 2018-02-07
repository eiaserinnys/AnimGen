#include "pch.h"
#include "ExponentialMap.h"

#include "exp-map.h"
#include "DXMathTransform.h"

using namespace Core;

//------------------------------------------------------------------------------
Vector3D ExponentialMap::FromQuaternion(const Vector4D& quat)
{
	static const double epsilon = sqrt(sqrt(0.00000001f));

	double theta = acos(quat.w) * 2;

	double sinHalfTheta = sin(theta / 2);

	double m = 0;

	if (abs(theta) < epsilon)
	{
		// ���Ϸ� ������ ���ؼ�
		m = 1 / (1.0 / 2 + theta * theta / 48);
	}
	else
	{
		m = theta / sinHalfTheta;
	}

	return Vector3D(m * quat.x, m * quat.y, m * quat.z);
}

//------------------------------------------------------------------------------
Vector4D ExponentialMap::ToQuaternion(const Vector3D& expMap)
{
	double quatV[4] = { 0, 0, 0, 0 };

	EM_To_Q((double*) expMap.m, quatV, 0);

	return Vector4D(quatV[0], quatV[1], quatV[2], quatV[3]);
}

//------------------------------------------------------------------------------
Matrix4D ExponentialMap::ToMatrix(const Vector3D& expMap)
{
	return DXMathTransform<double>::MatrixRotationQuaternion(
		ExponentialMap::ToQuaternion(expMap));
}