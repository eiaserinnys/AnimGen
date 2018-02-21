#include "pch.h"
#include "ExponentialMap.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "exp-map.h"
#include "DXMathTransform.h"

using namespace Core;

//------------------------------------------------------------------------------
Vector3D ExponentialMap::FromQuaternion(const Vector4D& quat_)
{
	static const double epsilon = sqrt(sqrt(0.00000001f));

	Vector4D quat =
		quat_.w < -1 || quat_.w > 1 ? Vector4D(Normalize(quat_)) : quat_;

	double theta = acos(quat.w) * 2;

	double sinHalfTheta = sin(theta / 2);

	double m = 0;

	if (abs(theta) < epsilon)
	{
		// 테일러 전개에 의해서
		m = 1 / (1.0 / 2 + theta * theta / 48);
	}
	else
	{
		m = theta / sinHalfTheta;
	}

	return Vector3D(m * quat.x, m * quat.y, m * quat.z);
}

//------------------------------------------------------------------------------
Vector3D ExponentialMap::Negate(const Vector3D& exp)
{
	double len = Length(exp);
	return len > 0 ? ((1 - 2 * M_PI / len) * exp).Evaluate() : exp;
}

//------------------------------------------------------------------------------
Vector3D ExponentialMap::GetNearRotation(const Vector3D& pivot, const Vector3D& toEval)
{
	Vector3D negated = Negate(toEval);
	double dist[] =
	{
		Distance(pivot, toEval),
		Distance(pivot, negated)
	};
	return dist[0] < dist[1] ? toEval : negated;
}

//------------------------------------------------------------------------------
void ExponentialMap::MakeNearRotation(const Vector3D& pivot, Vector3D& toEval)
{
	toEval = GetNearRotation(pivot, toEval);
}

//------------------------------------------------------------------------------
Vector4D ExponentialMap::ToQuaternion(const Vector3D& expMap, bool reg)
{
	double quatV[4] = { 0, 0, 0, 0 };

	EM_To_Q((double*) expMap.m, quatV, reg ? 1 : 0);

	return Vector4D(quatV[0], quatV[1], quatV[2], quatV[3]);
}

//------------------------------------------------------------------------------
Vector3D ExponentialMap::FromMatrix(const Matrix4D& mat)
{
	return ExponentialMap::FromQuaternion(
		DXMathTransform<double>::QuaternionRotationMatrix(mat));
}

//------------------------------------------------------------------------------
Matrix4D ExponentialMap::ToMatrix(const Vector3D& expMap)
{
	return DXMathTransform<double>::MatrixRotationQuaternion(
		ExponentialMap::ToQuaternion(expMap));
}