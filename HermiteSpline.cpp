#include "pch.h"
#include "HermiteSpline.h"

#include "Matrix.h"
#include "DXMathTransform.h"

using namespace std;
using namespace Core;

class HermiteSpline : public IHermiteSpline {
public:
	HermiteSpline(const vector<Vector3D>& p)
		: pos(p)
	{
		m = Matrix4D
		{
			+2, +1, -2, +1,
			-3, -2, +3, -1, 
			+0, +1, +0, +0, 
			+1, +0, +0, +0, 
		};
	}

	Vector3D At(double v)
	{
		int i = (int)v;
		double f = v - i;

		if (i < 0) { return P(0); }
		if (i >= pos.size()) { return *pos.rbegin(); }

		Vector4D u(f * f * f, f * f, f, 1);

		Vector4D um = DXMathTransform<double>::Transform(u, m);

		Vector3D Pi = P(i);
		Vector3D Ri = RightTangent(i);
		Vector3D Pi1 = P(i + 1);
		Vector3D Li1 = RightTangent(i + 1);
		MatrixT<double, 4, 3> p = 
		{
			Pi.x, Pi.y, Pi.z, 
			Ri.x, Ri.y, Ri.z, 
			Pi1.x, Pi1.y, Pi1.z, 
			Li1.x, Li1.y, Li1.z, 
		};

		return DXMathTransform<double>::Transform(um, p);
	}

	Vector3D P(int i) const
	{
		if (i < 0) { return pos[0]; }
		if (i >= pos.size()) { return *pos.rbegin(); }
		return pos[i];
	}

	Vector3D RightTangent(int i)
	{
		return 0.5 * (P(i + 1) - P(i - 1));
	}

	Matrix4D m;
	vector<Vector3D> pos;
};

IHermiteSpline::~IHermiteSpline()
{}

IHermiteSpline* IHermiteSpline::Create(const vector<Vector3D>& p)
{ return new HermiteSpline(p); }