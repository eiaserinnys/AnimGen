#include "pch.h"
#include "ClampedSplineFixedStep.h"

#include <exception>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <Utility.h>
#include <WindowsUtility.h>

#include "ExponentialMap.h"
#include "DXMathTransform.h"

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
		, solver(solver)
	{
		n = (int)(pos.size() - 1);
		te = n * timeStep;

		FillY(pos, ch);

		FillV();

		Solve();
	}

	void SetValue(int index, double v)
	{
		if (0 <= index && index < y.size())
		{
			y[index] = v;

			if (index - 1 >= 0)
			{
				(*V)(index - 1) = CalculateV(index - 1);
			}
			(*V)(index) = CalculateV(index);
			if (index + 1 < V->rows())
			{
				(*V)(index + 1) = CalculateV(index + 1);
			}
		}
		else
		{
			throw invalid_argument("");
		}
	}

	void Solve()
	{
		MatrixXd B = solver->solve(*V);
		if (solver->info() != Success) { throw runtime_error("SimplicialLLT::solve() failed"); }

		c.clear();
		c.reserve(n + 1);
		for (int i = 0; i < B.rows(); ++i) { c.push_back(B(i)); }

		//ValidateVelocity();

		//Test();
	}

	double Delta(int i)
	{
		if (i < 0) { return 0; }	// 시작 속도
		if (i >= n) { return 0; }	// 끝 속도
		return 1.0 / timeStep * (y[i + 1] - y[i]);
	}

	double CalculateV(int i)
	{
		return 3 * (Delta(i) - Delta(i - 1));
	}

	void FillV()
	{
		V.reset(new Matrix<double, Dynamic, 1>(n + 1));

		for (int i = 0; i <= n; ++i)
		{
			(*V)(i) = CalculateV(i);
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

	double AccelerationAt(double x_)
	{
		int i = (int)(x_ / timeStep);

		if (x_ < 0) { i = 0; }
		if (x_ >= te) { i = n - 1; }

		auto x = (x_ - i * timeStep);

		double h = timeStep;

		auto a = y[i];
		auto b = (1.0 / h * (y[i + 1] - y[i]) - h / 3 * (2 * c[i] + c[i + 1]));
		auto d = 1 / (3 * h) * (c[i + 1] - c[i]);

		return 2 * c[i] + 6 * d * x;
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

	SimplicialLLT<SparseMatrixD>* solver;
	unique_ptr<Matrix<double, Dynamic, 1>> V;
};

//------------------------------------------------------------------------------
class ClampedSplineFixedStep : public IClampedSplineFixedStep {
public:
	ClampedSplineFixedStep(
		const vector<Vector3D>& p,
		const vector<Vector3D>& r_,
		double timeStep)
		: rotOrg(r_)
		, timeStep(timeStep)
	{
		te = (p.size() - 1) * timeStep;

		assert(p.size() == r_.size());

		ComputeA(p.size() - 1, timeStep);

		channel[0].reset(new FixedSplineChannel(timeStep, p, 0, &solver));
		channel[1].reset(new FixedSplineChannel(timeStep, p, 1, &solver));
		channel[2].reset(new FixedSplineChannel(timeStep, p, 2, &solver));

		rotMod.clear();
		rotMod.reserve(rotOrg.size());

		for (int i = 0; i < rotOrg.size(); ++i)
		{
			rotMod.push_back(i > 0 ?
				ExponentialMap::GetNearRotation(rotMod[i - 1], rotOrg[i]) :
				rotOrg[0]);
		}

		channel[3].reset(new FixedSplineChannel(timeStep, rotMod, 0, &solver));
		channel[4].reset(new FixedSplineChannel(timeStep, rotMod, 1, &solver));
		channel[5].reset(new FixedSplineChannel(timeStep, rotMod, 2, &solver));
	}

	void SetValue(int index, int ch, double v)
	{
		if (0 <= channel && ch < COUNT_OF(channel))
		{
			if (ch < 3)
			{
				// 위치는 개별 채널만 수정 가능
				channel[ch]->SetValue(index, v);
				channel[ch]->Solve();
			}
			else
			{
				// 회전은 간단치 않다! 절차가 좀 까다로움
				bool invalidated[] = { false, false, false };
				
				// 일단 원하는 값을 수정하고
				rotOrg[index].m[ch - 3] = v;
				invalidated[ch - 3] = true;

				// 수정된 회전 키값부터 순회하면서
				for (int i = index; i < rotOrg.size(); ++i)
				{
					// 이전 키 값부터 봤을 때 가까운 회전 방향을 새로 구해서
					auto newRot = i > 0 ?
						ExponentialMap::GetNearRotation(rotMod[i - 1], rotOrg[i]) :
						rotOrg[0];

					// 그게 기존에 수정된 회전 방향과 다르면
					bool different = false;
					for (int j = 0; j < 3; ++j)
					{
						if (rotMod[i].m[j] != newRot.m[j])
						{
							// 해당 채널을 업데이트해준다
							channel[j + 3]->SetValue(i, newRot.m[j]);
							invalidated[j] = true;

							different = true;
						}
					}

					if (different)
					{
						rotMod[i] = newRot;
					}
				}

				// 최종적으로 수정된 적이 있는 채널은 모두 재계산한다
				for (int i = 0; i < COUNT_OF(invalidated); ++i)
				{
					channel[i + 3]->Solve();
				}
			}
		}
	}

	void ComputeA(int n, double timeStep)
	{
		SparseMatrixD A(n + 1, n + 1);

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

		solver.compute(A);
		if (solver.info() != Success) { throw runtime_error("SimplicialLLT::compute() failed"); }
	}

	pair<Vector3D, Vector3D> At(double v)
	{
		return make_pair(
			Vector3D(channel[0]->At(v), channel[1]->At(v), channel[2]->At(v)),
			Vector3D(channel[3]->At(v), channel[4]->At(v), channel[5]->At(v)));
	}

	pair<Vector3D, Vector3D> AccelerationAt(double v)
	{
		return make_pair(
			Vector3D(channel[0]->AccelerationAt(v), channel[1]->AccelerationAt(v), channel[2]->AccelerationAt(v)),
			Vector3D(channel[3]->AccelerationAt(v), channel[4]->AccelerationAt(v), channel[5]->AccelerationAt(v)));
	}

	double GetMax()
	{
		return te;
	}

	double timeStep;
	vector<Vector3D> rotOrg;
	vector<Vector3D> rotMod;

	double te;

	unique_ptr<FixedSplineChannel> channel[6];

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

//------------------------------------------------------------------------------
void TestCurve()
{
	for (int i = 0; i < 10 * 6; ++i)
	{
		//WindowsUtility::Debug(L"Trial %d,%d\n", i/6, i%6);

		vector<Vector3D> posA, posB;
		vector<Vector3D> rotA, rotB;

		double preserved = 0;

		for (int j = 0; j <= 10; ++j)
		{
			auto p = Vector3D(j, j % 2 ? -1 : 1, j % 2 ? 1 : -1);
			auto r = j % 2 ?
				Vector3D(0, 0, 0) :
				ExponentialMap::FromMatrix(
					DXMathTransform<double>::RotationY(90.0 / 180 * M_PI) *
					DXMathTransform<double>::RotationZ(45.0 / 180 * M_PI));

			posA.push_back(p);
			rotA.push_back(r);

			int ei = i / 6;
			int ej = i % 6;

			if (ei == j)
			{
				switch (ej) {
				case 0: preserved = p.m[0]; p.m[0] = 1000; break;
				case 1: preserved = p.m[1]; p.m[1] = 1000; break;
				case 2: preserved = p.m[2]; p.m[2] = 1000; break;
				case 3: preserved = r.m[0]; r.m[0] = 1000; break;
				case 4: preserved = r.m[1]; r.m[1] = 1000; break;
				case 5: preserved = r.m[2]; r.m[2] = 1000; break;
				}
			}

			posB.push_back(p);
			rotB.push_back(r);
		}

		unique_ptr<ISpline> a(IClampedSplineFixedStep::Create(1, posA, rotA));
		unique_ptr<ISpline> b(IClampedSplineFixedStep::Create(1, posB, rotB));

		for (double t = 0; t < 10; t += 0.1)
		{
			auto av = a->At(t);
			auto bv = b->At(t);

			//WindowsUtility::Debug(
			//	L"(%f)(%f,%f,%f,%f,%f,%f)(%f,%f,%f,%f,%f,%f)\n",
			//	t,
			//	av.first.x, av.first.y, av.first.z,
			//	av.second.x, av.second.y, av.second.z,
			//	bv.first.x, bv.first.y, bv.first.z,
			//	bv.second.x, bv.second.y, bv.second.z);

			//assert(av.first.x == bv.first.x);
			//assert(av.first.y == bv.first.y);
			//assert(av.first.z == bv.first.z);

			//assert(av.second.x == bv.second.x);
			//assert(av.second.y == bv.second.y);
			//assert(av.second.z == bv.second.z);
		}

		int ei = i / 6;
		int ej = i % 6;

		b->SetValue(ei, ej, preserved);

		for (double t = 0; t < 10; t += 0.1)
		{
			auto av = a->At(t);
			auto bv = b->At(t);

			//WindowsUtility::Debug(
			//	L"(%f)(%f,%f,%f,%f,%f,%f)(%f,%f,%f,%f,%f,%f)\n",
			//	t,
			//	av.first.x, av.first.y, av.first.z,
			//	av.second.x, av.second.y, av.second.z,
			//	bv.first.x, bv.first.y, bv.first.z,
			//	bv.second.x, bv.second.y, bv.second.z);

			assert(av.first.x == bv.first.x);
			assert(av.first.y == bv.first.y);
			assert(av.first.z == bv.first.z);

			assert(av.second.x == bv.second.x);
			assert(av.second.y == bv.second.y);
			assert(av.second.z == bv.second.z);
		}

		//WindowsUtility::Debug(L"\n");
	}
}