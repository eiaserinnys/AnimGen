#pragma once

#include <DirectXMath.h>
#include "Vector.h"

namespace Core
{
	template <typename V, int D, ENABLE_IF(D >= 3)>
	inline DirectX::XMFLOAT3 ToXMFLOAT3(const VectorT<V, D>& v)
	{
		return DirectX::XMFLOAT3((V)v.x, (V)v.y, (V)v.z);
	}

	inline Vector3F FromXMFLOAT3(const DirectX::XMFLOAT3& v)
	{
		return Vector3F(v.x, v.y, v.z);
	}
};