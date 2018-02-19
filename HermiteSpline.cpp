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
			+2, +1, -2, +1,
			-3, -2, +3, -1, 
			+0, +1, +0, +0, 
			+1, +0, +0, +0, 
		};
	}

	Vector3D RightTangent(int i)
	{
		return 0.5 * (P(i + 1) - P(i - 1));
	}

	Vector3D SphericalTangent(int i)
	{
		return SphericalTangent(R(i - 1), R(i + 1));
	}

	Vector3D SphericalTangent(const Vector3D& em1, const Vector3D& ep1)
	{
		auto qp1 = ExponentialMap::ToQuaternion(ep1);
		auto qm1 = ExponentialMap::ToQuaternion(em1);

		auto q = DXMathTransform<double>::QuaternionMultiply(
			DXMathTransform<double>::QuaternionInverse(qm1),
			qp1);

		return 0.5 * ExponentialMap::FromQuaternion(q);
	}

	pair<Vector3D, Vector3D> At(double v)
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

		auto Pi = P(i);
		auto Ri = RightTangent(i);
		auto Pi1 = P(i + 1);
		auto Li1 = RightTangent(i + 1);

		auto Ei = R(i);
		auto Eri = SphericalTangent(i);
		auto Ei1 = R(i + 1);
		auto El1 = SphericalTangent(i + 1);

		Vector3D eSrc[] = { R(i - 1), R(i), R(i + 1), R(i + 2), };
		Vector3D e[4];

		e[0] = ExponentialMap::GetNearRotation(eSrc[1], eSrc[0]);
		e[1] = eSrc[1];
		e[2] = ExponentialMap::GetNearRotation(eSrc[1], eSrc[2]);
		e[3] = ExponentialMap::GetNearRotation(e[2], eSrc[3]);

		Ei = e[1];
		Eri = SphericalTangent(e[0], e[2]);
		Ei1 = e[2];
		El1 = SphericalTangent(e[1], e[3]);

		return make_pair(
			Vector3D(um.x * Pi + um.y * Ri + um.z * Pi1 + um.w * Li1),
			Vector3D(um.x * Ei + um.y * Eri + um.z * Ei1 + um.w * El1));
	}

	Vector3D P(int i) const
	{
		if (i < 0) { return pos[0]; }
		if (i + 1 >= pos.size()) { return *pos.rbegin(); }
		return pos[i];
	}

	Vector3D R(int i) const
	{
		if (i < 0) { return rot[0]; }
		if (i + 1 >= rot.size()) { return *rot.rbegin(); }
		return rot[i];
	}

	double GetMax() { return pos.size() - 1; }

	Matrix4D m;
	vector<Vector3D> pos;
	vector<Vector3D> rot;		// exponential map
};

IHermiteSpline::~IHermiteSpline()
{}

IHermiteSpline* IHermiteSpline::Create(
	const vector<Vector3D>& p,
	const vector<Vector3D>& r)
{ return new HermiteSpline(p, r); }