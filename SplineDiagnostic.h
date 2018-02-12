#pragma once

#include <DirectXMath.h>

#include "HermiteSpline.h"
#include "Vector.h"
#include "Matrix.h"

class LineBuffer;

//------------------------------------------------------------------------------
struct SplineDiagnostic
{
	std::unique_ptr<IHermiteSpline> spline;
	std::vector<Core::Vector3D> pos;
	std::vector<Core::Vector3D> rot;
	std::vector<DirectX::XMMATRIX> tx;

	std::vector<DirectX::XMFLOAT3> sampled;

	SplineDiagnostic(
		const std::vector<Core::Vector3D>& pos, 
		const std::vector<Core::Vector3D>& rot);

	void Sample(int g);

	void Enqueue(LineBuffer* lineBuffer, double factor);
};
