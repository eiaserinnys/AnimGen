#include "pch.h"
#include "ClampedSpline.h"

#include <exception>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <WindowsUtility.h>

#include "ExponentialMap.h"

using namespace std;
using namespace Core;
using namespace Eigen;

typedef Triplet<double> TripletD;
typedef SparseMatrix<double> SparseMatrixD;

// Clamped Spline
// http://macs.citadel.edu/chenm/343.dir/09.dir/lect3_4.pdf

//------------------------------------------------------------------------------
struct SplineChannel
{
	SplineChannel(
		const vector<double>& t,
		const vector<Vector3D>& pos,
		int ch)
		: t(t)
	{
		n = (int) (pos.size() - 1);

		FillY(pos, ch);
		
		FillH();

		SparseMatrixD A;
		FillA(A);

		Matrix<double, Dynamic, 1> V;
		FillV(V);

		solver.compute(A);
		if (solver.info() != Success) { throw runtime_error("SimplicialLLT::compute() failed"); }

		MatrixXd B = solver.solve(V);
		if (solver.info() != Success) { throw runtime_error("SimplicialLLT::solve() failed"); }

		c.reserve(n + 1);

		for (int i = 0; i < V.rows(); ++i)
		{
			c.push_back(V(i));
		}
	}

	void FillA(SparseMatrixD& A)
	{
		A = SparseMatrixD(n + 1, n + 1);

		vector<TripletD> tp;
		for (int i = 0; i <= n; ++i)
		{
			auto& H = [&](int i)
			{
				if (i <= 0) { return h[0]; }
				if (i >= h.size()) { return *h.rbegin(); }
				return h[i];
			};

			if (i > 0 && i < n)
			{
				tp.push_back(TripletD(i, i, 2 * (H(i - 1) + H(i))));
			}
			else
			{ 
				tp.push_back(TripletD(i, i, 2 * H(i)));
			}

			if (i < n)
			{
				tp.push_back(TripletD(i, i + 1, h[i]));
				tp.push_back(TripletD(i + 1, i, h[i]));
			}
		}

		A.setFromTriplets(tp.begin(), tp.end());
	}

	void FillV(Matrix<double, Dynamic, 1>& V)
	{
		V = Matrix<double, Dynamic, 1>(n + 1);

		for (int i = 0; i <= n; ++i)
		{
			auto& Delta = [&](int i) -> double
			{
				if (i < 0) { return 0; }	// 시작 속도
				if (i >= n) { return 0; }	// 끝 속도
				return 1.0 / h[i] * (y[i + 1] - y[i]);
			};

			V(i) = 3 * (Delta(i) - Delta(i - 1));
		}
	}

	void FillY(const vector<Vector3D>& pos, int ch)
	{
		y.clear();
		y.reserve(pos.size());
		for (int i = 0; i < pos.size(); ++i)
		{
			y.push_back(pos[i].m[ch]);
		}
	}

	void FillH()
	{
		h.clear();
		h.reserve(n);
		for (int i = 0; i <= n - 1; ++i)
		{
			h.push_back(t[i + 1] - t[i]);
		}
	}

	double At(double x)
	{
		if (x < t[0]) { return y[0]; }
		if (x >= *t.rbegin()) { return *y.rbegin(); }

		for (int i = 0; i < t.size() - 1; ++i)
		{
			if (t[i] <= x && x <= t[i + 1])
			{
				auto d = (x - t[i]);

				return
					y[i] +
					(1.0 / h[i] * (y[i + 1] - y[i]) - h[i] / 3 * (2 * c[i] + c[i + 1])) * d +
					c[i] * d * d +
					1 / (3 * h[i]) * (c[i + 1] - c[i]) * d * d * d;
			}
		}

		return y[0];
	}

	void Test()
	{
		double f = t[0], e = *t.rbegin();

		for (double x = f; x < e; x += 0.01)
		{
			WindowsUtility::Debug(L"%f, %f\n", x, At(x));
		}
	}

	int n;
	vector<double> t;
	vector<double> y;
	vector<double> h;
	vector<double> c;

	SimplicialLLT<SparseMatrixD> solver;
};

//------------------------------------------------------------------------------
class ClampedSpline : public IClampedSpline {
public:
	ClampedSpline(
		const vector<double>& t, 
		const vector<Vector3D>& p, 
		const vector<Vector3D>& r_)
	{
		ts = t[0];
		te = *t.rbegin();

		assert(t.size() == p.size());
		assert(p.size() == r_.size());

		pos[0].reset(new SplineChannel(t, p, 0));
		pos[1].reset(new SplineChannel(t, p, 1));
		pos[2].reset(new SplineChannel(t, p, 2));

		vector<Vector3D> r;
		for (int i = 0; i < r_.size(); ++i)
		{
			if (i > 0)
			{
				r.push_back(ExponentialMap::GetNearRotation(r[i - 1], r_[i]));
			}
			else
			{
				r.push_back(r_[0]);
			}
		}

		rot[0].reset(new SplineChannel(t, r, 0));
		rot[1].reset(new SplineChannel(t, r, 1));
		rot[2].reset(new SplineChannel(t, r, 2));
	}

	pair<Vector3D, Vector3D> At(double v)
	{
		return make_pair(
			Vector3D(
				pos[0]->At(v),
				pos[1]->At(v),
				pos[2]->At(v)),
			Vector3D(
				rot[0]->At(v),
				rot[1]->At(v),
				rot[2]->At(v))
		);
	}

	double GetMax()
	{
		return te;
	}

	double ts, te;
	unique_ptr<SplineChannel> pos[3];
	unique_ptr<SplineChannel> rot[3];
};

//------------------------------------------------------------------------------
ISpline::~ISpline()
{}

//------------------------------------------------------------------------------
IClampedSpline* IClampedSpline::Create(
	const vector<double>& t,
	const vector<Vector3D>& p,
	const vector<Vector3D>& r)
{ return new ClampedSpline(t, p, r); }