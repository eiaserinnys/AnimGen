#pragma once

#include <DirectXMath.h>

#include "Spline.h"
#include "Vector.h"
#include "Matrix.h"

class LineBuffer;

//------------------------------------------------------------------------------
struct SplineDiagnostic
{
	std::vector<DirectX::XMMATRIX> tx;
	std::vector<DirectX::XMFLOAT3> sampled;

	void Sample(ISpline* spline, int g);
	void Enqueue(ISpline* spline, LineBuffer* lineBuffer, double factor);
};
