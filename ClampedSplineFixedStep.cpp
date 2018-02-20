#include "pch.h"
#include "ClampedSplineFixedStep.h"

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
struct FixedSplineChannel
{
	FixedSplineChannel(
		double timeStep, 
		const vector<Vector3D>& pos,
		int ch, 
		SimplicialLLT<SparseMatrixD>* solver)
		: timeStep(timeStep)
	{
		n = (int)(pos.size() - 1);
		te = n * timeStep;

		FillY(pos, ch);

		Matrix<double, Dynamic, 1> V;
		FillV(V);

		MatrixXd B = solver->solve(V);
		if (solver->info() != Success) { throw runtime_error("SimplicialLLT::solve() failed"); }

		c.reserve(n + 1);
		for (int i = 0; i < B.rows(); ++i)
		{
			c.push_back(B(i));
		}

		//ValidateVelocity();

		//Test();
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
				return 1.0 / timeStep * (y[i + 1] - y[i]);
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

	double At(double x_)
	{
		int i = (int)(x_ / timeStep);

		if (x_ < 0) { i = 0; }
		if (x_ >= te) { i = n - 1; }

		auto x = (x_ - i * timeStep);

		double h = timeStep;

		auto a = y[i];
		auto b = (1.0 / h * (y[i + 1] - y[i]) - h / 3 * (2 * c[i] + c[i + 1]));
		auto d = 1 / (3 * h) * (c[i + 1] - c[i]);

		return a + b * x + c[i] * x * x + d * x * x * x;
	}

	void ValidateVelocity()
	{
		double h = timeStep;

		for (int i = 0; i < n; ++i)
		{
			auto a = y[i];
			auto b = (1.0 / h * (y[i + 1] - y[i]) - h / 3 * (2 * c[i] + c[i + 1]));
			auto d = 1 / (3 * h) * (c[i + 1] - c[i]);

			auto t0 = i * h;
			auto t1 = (i + 1) * h;

			auto v1 = b + 2 * c[i] * h + 3 * d * h * h;

			WindowsUtility::Debug(
				L"S'%d(%f)=%f, S'%d(%f)=%f\n", 
				i, t0, b, 
				i, t1, v1);
		}
	}

	void Test()
	{
		double f = 0, e = te;

		for (double x = f; x < e; x += 0.01)
		{
			WindowsUtility::Debug(L"%f, %f\n", x, At(x));
		}
	}

	int n;
	double te;
	double timeStep;
	vector<double> y;
	vector<double> c;
};

//------------------------------------------------------------------------------
class ClampedSplineFixedStep : public IClampedSplineFixedStep {
public:
	ClampedSplineFixedStep(
		const vector<Vector3D>& p,
		const vector<Vector3D>& r_, 
		double timeStep)
	{
		te = (p.size() - 1) * timeStep;

		assert(p.size() == r_.size());

		SparseMatrixD A;
		FillA(A, p.size() - 1, timeStep);

		solver.compute(A);
		if (solver.info() != Success) { throw runtime_error("SimplicialLLT::compute() failed"); }

		pos[0].reset(new FixedSplineChannel(timeStep, p, 0, &solver));
		pos[1].reset(new FixedSplineChannel(timeStep, p, 1, &solver));
		pos[2].reset(new FixedSplineChannel(timeStep, p, 2, &solver));

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

		rot[0].reset(new FixedSplineChannel(timeStep, r, 0, &solver));
		rot[1].reset(new FixedSplineChannel(timeStep, r, 1, &solver));
		rot[2].reset(new FixedSplineChannel(timeStep, r, 2, &solver));
	}

	void FillA(SparseMatrixD& A, int n, double timeStep)
	{
		A = SparseMatrixD(n + 1, n + 1);

		vector<TripletD> tp;
		for (int i = 0; i <= n; ++i)
		{
			if (i > 0 && i < n)
			{
				tp.push_back(TripletD(i, i, 2 * (timeStep + timeStep)));
			}
			else
			{
				tp.push_back(TripletD(i, i, 2 * timeStep));
			}

			if (i < n)
			{
				tp.push_back(TripletD(i, i + 1, timeStep));
				tp.push_back(TripletD(i + 1, i, timeStep));
			}
		}

		A.setFromTriplets(tp.begin(), tp.end());
	}

	pair<Vector3D, Vector3D> At(double v)
	{
		return make_pair(
			Vector3D(pos[0]->At(v), pos[1]->At(v), pos[2]->At(v)),
			Vector3D(rot[0]->At(v), rot[1]->At(v), rot[2]->At(v)));
	}

	double GetMax()
	{
		return te;
	}

	double te;
	unique_ptr<FixedSplineChannel> pos[3];
	unique_ptr<FixedSplineChannel> rot[3];

	SimplicialLLT<SparseMatrixD> solver;
};

//------------------------------------------------------------------------------
IClampedSplineFixedStep* IClampedSplineFixedStep::Create(
	double timeStep, 
	const vector<Vector3D>& p,
	const vector<Vector3D>& r)
{
	return new ClampedSplineFixedStep(p, r, timeStep);
}