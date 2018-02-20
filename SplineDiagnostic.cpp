#include "pch.h"
#include "SplineDiagnostic.h"

#include <WindowsUtility.h>

#include "ExponentialMap.h"
#include "FrameHelper.h"
#include "DXMathTransform.h"
#include "LineBuffer.h"

using namespace std;
using namespace Core;
using namespace DirectX;

//------------------------------------------------------------------------------
XMMATRIX ToXMMATRIX(const Matrix4D& m)
{
	XMMATRIX xm;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			xm.r[i].m128_f32[j] = m.m[i][j];
		}
	}
	return xm;
}

//------------------------------------------------------------------------------
void SplineDiagnostic::Sample(ISpline* spline, int g)
{
	double m = spline->GetMax();

	{
		tx.clear();

		for (size_t i = 0; i < m; ++i)
		{
			auto ret = spline->At(i);

			Vector4D q = ExponentialMap::ToQuaternion(ret.second);
			Matrix4D m = DXMathTransform<double>::MatrixRotationQuaternion(q);
			FrameHelper::SetTranslation(m, ret.first);

			tx.push_back(ToXMMATRIX(m));
		}
	}

	{
		sampled.clear();

		for (int i = 0; i <= m* g; ++i)
		{
			auto ret = spline->At((double)i / g);
			auto p = ret.first;
			sampled.push_back(XMFLOAT3(p.x, p.y, p.z));
		}
	}
}

//------------------------------------------------------------------------------
void SplineDiagnostic::Enqueue(
	ISpline* spline,
	LineBuffer* lineBuffer, 
	double t)
{
	for (auto it = tx.begin(); it != tx.end(); ++it)
	{
		lineBuffer->EnqueueFrame(*it, 0.05f);
	}

	{
		auto ret = spline->At(t);

		auto m = DXMathTransform<double>::MatrixRotationQuaternion(
			ExponentialMap::ToQuaternion(ret.second));
		FrameHelper::SetTranslation(m, ret.first);

		XMMATRIX xm;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				xm.r[i].m128_f32[j] = m.m[i][j];
			}
		}

		lineBuffer->EnqueueFrame(xm, 0.25f);
	}

	lineBuffer->Enqueue(sampled, 0xff00ffff);
}
