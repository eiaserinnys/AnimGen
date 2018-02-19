#pragma once

#include <DirectXMath.h>

#include "HermiteSpline.h"
#include "Vector.h"
#include "Matrix.h"

class LineBuffer;

//------------------------------------------------------------------------------
struct SplineDiagnostic
{
	std::vector<DirectX::XMMATRIX> tx;
	std::vector<DirectX::XMFLOAT3> sampled;

	void Sample(IHermiteSpline* spline, int g);
	void Enqueue(IHermiteSpline* spline, LineBuffer* lineBuffer, double factor);
};
