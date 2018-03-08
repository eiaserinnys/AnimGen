#include "pch.h"
#include "HermiteSpline.h"

#include <WindowsUtility.h>

#include "Matrix.h"
#include "ExponentialMap.h"
#include "DXMathTransform.h"

using namespace std;
using namespace Core;

class HermiteSpline : public IHermiteSpline {
public:
	HermiteSpline(const vector<Vector3D>& p, const vector<Vector3D>& r)
		: pos(p), rot(r)
	{
		assert(pos.size() == rot.size());

		m = Matrix4D
		{
			+2, -2, +1, +1,
			-3, +3, -2, -1, 
			+0, +0, +1, +0, 
			+1, +0, +0, +0, 
		};
	}

	Vector3D RightTangent(int i)
	{
		return 0.5 * (P(i + 1) - P(i - 1));
		//return (P(i + 1) - P(i));
	}

	Vector3D SphericalTangent(const Vector3D& em1, const Vector3D& ep1)
	{
		auto qp1 = ExponentialMap::ToQuaternion(ep1);
		auto qm1 = ExponentialMap::ToQuaternion(em1);

		auto q = DXMathTransform<double>::QuaternionMultiply(
			DXMathTransform<double>::QuaternionInverse(qm1),
			qp1);

		return 0.5 * ExponentialMap::FromQuaternion(q);
		//return ExponentialMap::FromQuaternion(q);
	}

	PositionRotation At(double v)
	{
		int i = (int) v;
		double f = v - i;

		if (v < 0) 
		{ 
			if (i < 0) 
			{ 
				throw std::invalid_argument("argument too small"); 
			}
		}
		if (i + 1 >= pos.size()) 
		{ 
			if (i >= pos.size())
			{
				throw std::invalid_argument("argument too large");
			}
		}

		Vector4D u(f * f * f, f * f, f, 1);

		Vector4D um = DXMathTransform<double>::Transform(u, m);

		auto p1 = P(i);
		auto dp1 = RightTangent(i);
		auto p2 = P(i + 1);
		auto dp2 = RightTangent(i + 1);

		Vector3D eSrc[] = { R(i - 1), R(i), R(i + 1), R(i + 2), };
		Vector3D e[4];

		e[0] = ExponentialMap::GetNearRotation(eSrc[1], eSrc[0]);
		e[1] = eSrc[1];
		e[2] = ExponentialMap::GetNearRotation(eSrc[1], eSrc[2]);
		e[3] = ExponentialMap::GetNearRotation(e[2], eSrc[3]);

		auto r1 = e[1];
		auto dr1 = SphericalTangent(e[0], e[2]);
		auto r2 = e[2];
		auto dr2 = SphericalTangent(e[1], e[3]);

		return PositionRotation
		{
			Vector3D(um.x * p1 + um.y * p2 + um.z * dp1 + um.w * dp2),
			Vector3D(um.x * r1 + um.y * r2 + um.z * dr1 + um.w * dr2)
		};
	}

	Vector3D P(int i) const
	{
		if (i <= 0) { return pos[0]; }
		if (i + 1 >= pos.size()) { return *pos.rbegin(); }
		return pos[i];
	}

	Vector3D R(int i) const
	{
		if (i <= 0) { return rot[0]; }
		if (i + 1 >= rot.size()) { return *rot.rbegin(); }
		return rot[i];
	}

	double GetMax() { return pos.size() - 1; }

	Matrix4D m;
	vector<Vector3D> pos;
	vector<Vector3D> rot;		// exponential map
};

IHermiteSpline* IHermiteSpline::Create(
	const vector<Vector3D>& p,
	const vector<Vector3D>& r)
{ return new HermiteSpline(p, r); }