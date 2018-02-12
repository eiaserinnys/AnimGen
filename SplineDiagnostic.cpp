#include "pch.h"
#include "SplineDiagnostic.h"

#include <timeapi.h>

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
SplineDiagnostic::SplineDiagnostic(
	const vector<Vector3D>& pos, 
	const vector<Vector3D>& rot)
	: pos(pos), 
	rot(rot)
{
	spline.reset(IHermiteSpline::Create(pos, rot));

	for (size_t i = 0; i < pos.size(); ++i)
	{ 
		Vector4D q = ExponentialMap::ToQuaternion(rot[i]);
		Matrix4D m = DXMathTransform<double>::MatrixRotationQuaternion(q);
		FrameHelper::SetTranslation(m, pos[i]);

		tx.push_back(ToXMMATRIX(m));
	}

	Sample(20);
}

//------------------------------------------------------------------------------
void SplineDiagnostic::Sample(int g)
{
	for (int i = 0; i <= pos.size() * g; ++i)
	{
		auto ret = spline->At((double)i / g);
		auto p = ret.first;
		sampled.push_back(XMFLOAT3(p.x, p.y, p.z));
	}
}

//------------------------------------------------------------------------------
void SplineDiagnostic::Enqueue(LineBuffer* lineBuffer)
{
	auto curTime = timeGetTime();
	if (lastTime != 0)
	{
		elapsed += curTime - lastTime;
		lastTime = curTime;
	}
	else
	{
		elapsed = 0;
		lastTime = curTime;
	}

	elapsed = elapsed % 30000;

	double factor = (elapsed / 30000.0) * points;

	for (auto it = tx.begin(); it != tx.end(); ++it)
	{
		lineBuffer->EnqueueFrame(*it, 0.05f);
	}

	{
		auto ret = spline->At(factor);

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
