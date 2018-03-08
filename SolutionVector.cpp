#include "pch.h"
#include "SolutionVector.h"

#include <stdio.h>

#include <Utility.h>
#include <WindowsUtility.h>

#include "ExponentialMap.h"
#include "DXMathTransform.h"

#include "Robot.h"
#include "ClampedSplineFixedStep.h"
#include "SolutionSpline.h"

using namespace std;
using namespace Core;

const double g_timeStep = 0.5;

//------------------------------------------------------------------------------
class SolutionVector : public ISolutionVector {
public:
	//--------------------------------------------------------------------------
	SolutionVector(vector<pair<double, SolutionCoordinate>>& coords)
		: coords(coords)
	{
		robot.reset(IRobot::Create());

		UpdateSpline();
	}

	//--------------------------------------------------------------------------
	void UpdateSpline()
	{
		for (int i = 0; i < COUNT_OF(splines); ++i)
		{
			splines[i].Clear();
		}

		for (auto it = coords.begin(); it != coords.end(); ++it)
		{
			auto coord = it->second;

			splines[0].Append(it->first, coord.body);
			splines[1].Append(it->first, coord.foot[0]);
			splines[2].Append(it->first, coord.foot[1]);
		}

		for (int i = 0; i < COUNT_OF(splines); ++i)
		{
			splines[i].Update();
		}
	}

	//--------------------------------------------------------------------------
	SolutionVector(const SolutionVector& rhs)
	{
		robot.reset(IRobot::Create());
		coords = rhs.coords;
		for (int i = 0; i < COUNT_OF(splines); ++i)
		{
			splines[i] = rhs.splines[i];
		}
	}

	//------------------------------------------------------------------------------
	ISolutionVector* Clone() const
	{ return new SolutionVector(*this); }

	//--------------------------------------------------------------------------
	void ValidatePhaseRange() const
	{ if (coords.empty()) { throw out_of_range(""); } }

	//--------------------------------------------------------------------------
	void ValidatePhaseRange(int i) const
	{ if (i < 0 || i >= coords.size()) { throw out_of_range(""); } }

	//--------------------------------------------------------------------------
	int GetPhaseCount() const
	{ return (int)coords.size(); }

	//--------------------------------------------------------------------------
	double GetPhaseTime(int i) const
	{
		ValidatePhaseRange(i);
		return coords[i].first;
	}

	//--------------------------------------------------------------------------
	double GetLastPhaseTime() const
	{
		ValidatePhaseRange();
		return coords.rbegin()->first;
	}

	//--------------------------------------------------------------------------
	SolutionCoordinate& GetPhase(int i)
	{
		ValidatePhaseRange(i);
		return coords[i].second;
	}

	//--------------------------------------------------------------------------
	const SolutionCoordinate& GetPhase(int i) const
	{
		return const_cast<SolutionVector*>(this)->GetPhase(i);
	}

	//--------------------------------------------------------------------------
	SolutionCoordinate& GetLastPhase()
	{
		ValidatePhaseRange();
		return coords.rbegin()->second;
	}

	//--------------------------------------------------------------------------
	const SolutionCoordinate& GetLastPhase() const
	{
		return const_cast<SolutionVector*>(this)->GetLastPhase();
	}

	//--------------------------------------------------------------------------
	ISpline* GetCurve(int i)
	{
		if (0 <= i && i < COUNT_OF(splines))
		{
			return splines[i].curve.get();
		}
		return nullptr;
	}

	//--------------------------------------------------------------------------
	int VariableCount() const
	{
		return coords.empty() ?
			0 :
			(int)(coords.size() - 1) * SolutionCoordinate::VariableCount();
	}

	//--------------------------------------------------------------------------
	pair<int, int> GetVariableIndex(int i) const
	{
		int varPerEntry = SolutionCoordinate::VariableCount();

		i = i + varPerEntry;

		if (0 <= i && i < varPerEntry * coords.size())
		{
			int ei = i / varPerEntry;
			int ej = i % varPerEntry;
			return make_pair(ei, ej);
		}

		throw invalid_argument("");
	}

	//--------------------------------------------------------------------------
	double GetVariableAt(int i) const
	{
		auto index = GetVariableIndex(i);
		return coords[index.first].second.At(index.second);
	}

	//--------------------------------------------------------------------------
	void SetVariableAt(int i, double v)
	{
		auto index = GetVariableIndex(i);
		coords[index.first].second.At(index.second) = v;

		// 커브를 업데이트한다
		int ci = index.second / 6;
		int co = index.second % 6;
		splines[ci].SetValue(index.first, co, v);
	}

	//--------------------------------------------------------------------------
	SolutionCoordinate At(double t) const
	{
		SolutionCoordinate c;

		c.body = splines[0].curve->At(t);
		c.foot[0] = splines[1].curve->At(t);
		c.foot[1] = splines[2].curve->At(t);

		return c;
	}

	void EnableIKDump(bool enable, bool left)
	{
		robot->EnableIKDump(enable, left);
	}

	//--------------------------------------------------------------------------
	GeneralCoordinate GeneralCoordinateAt(double t, bool dump = false) const
	{
		SolutionCoordinate sc = At(t);

		//if (dump) { sc.Dump(); }

		robot->Apply(sc, dump);

		return robot->Current();
	}

	//--------------------------------------------------------------------------
	GeneralCoordinate GeneralAccelerationAt(double t, bool highOrder, bool dump) const
	{
		const double derStep = 0.001;

		if (highOrder)
		{
			SolutionCoordinate sc[] =
			{
				At(t - derStep * 2),
				At(t - derStep),
				At(t),
				At(t + derStep),
				At(t + derStep * 2),
			};

			GeneralCoordinate gc[5];

			for (int i = 0; i < COUNT_OF(gc); ++i)
			{
				robot->Apply(sc[i]);
				gc[i] = robot->Current();

				if (dump)
				{
					WindowsUtility::Debug(L"\t\tgc[%d]=%f\n", i, gc[i].leg[0].rot1.z);
				}
			}

			int pivot = COUNT_OF(gc) / 2;
			for (int i = 0; i < COUNT_OF(gc); ++i)
			{
				if (i != pivot)
				{
					gc[i].MakeNear(gc[pivot]);
				}
			}

			// http://www.mathematik.uni-dortmund.de/~kuzmin/cfdintro/lecture4.pdf
			return (
				gc[4] * (-1.0) +
				gc[3] * 16.0 +
				gc[2] * (-30.0) +
				gc[1] * 16.0 +
				gc[0] * (-1.0)) / (12 * derStep * derStep);
		}
		else
		{
			GeneralCoordinate gc[3] =
			{
				GeneralCoordinateAt(t - derStep),
				GeneralCoordinateAt(t),
				GeneralCoordinateAt(t + derStep),
			};

			if (dump)
			{
				for (int i = 0; i < COUNT_OF(gc); ++i)
				{
					WindowsUtility::Debug(
						L"\t\tgc[%d]=%f, %f\n",
						i,
						gc[i].leg[0].rot1.z,
						gc[i].leg[0].rot1.z / M_PI * 180);
				}
			}

			int pivot = COUNT_OF(gc) / 2;
			for (int i = 0; i < COUNT_OF(gc); ++i)
			{
				if (i != pivot)
				{
					gc[i].MakeNear(gc[pivot]);
				}
			}

			// https://www.scss.tcd.ie/~dahyotr/CS7ET01/01112007.pdf
			return (gc[2] - (gc[1] * 2.0) + gc[0]) / (derStep * derStep);
		}
	}

	void Dump();

public:
	// 솔루션 벡터
	vector<pair<double, SolutionCoordinate>> coords;

	// 솔루션 벡터 -> 스플라인
	SolutionSpline splines[3];

	// 일반화 좌표 샘플링
	vector<pair<double, GeneralCoordinate>> gcSamples;

	// 내부용 로봇
	unique_ptr<IRobot> robot;
};

//------------------------------------------------------------------------------
ISolutionVector::~ISolutionVector() = default;

//------------------------------------------------------------------------------
double ISolutionVector::Timestep() { return g_timeStep; }

//------------------------------------------------------------------------------
ISolutionVector* ISolutionVector::Create(
	const SolutionCoordinate& init,
	int phases)
{
	vector<pair<double, SolutionCoordinate>> coords;

	srand(0);

	auto& noise = []()
	{
		return (((double)rand() / RAND_MAX) * 2 - 1) * 0.0001;
	};

	for (int i = 0; i <= phases; ++i)
	{
		SolutionCoordinate sc = init;

		sc.body.first += Vector3D(noise(), noise(), noise());
		sc.body.second += Vector3D(noise(), noise(), noise());
		sc.foot[0].first += Vector3D(noise(), noise(), noise());
		sc.foot[0].second += Vector3D(noise(), noise(), noise());
		sc.foot[1].first += Vector3D(noise(), noise(), noise());
		sc.foot[1].second += Vector3D(noise(), noise(), noise());

		coords.push_back(make_pair(i * g_timeStep, sc));
	}

	return new SolutionVector(coords);
}

//------------------------------------------------------------------------------
ISolutionVector* ISolutionVector::BuildTest(const SolutionCoordinate& init)
{
	auto delta = Vector3D(2, 0, 0);
	auto deltaF = Vector3D(1.5, 0.0f, 0);
	auto deltaF2 = Vector3D(3, 0, 0);

	int phases = 8;

	vector<pair<double, SolutionCoordinate>> coords;

	for (int i = 0; i <= phases; ++i)
	{
		double factor = (double)i / phases;

		SolutionCoordinate nc = init;

		double d = 0;
		d = i % 2 == 0 ? 0.025 : -0.025;

		auto move = Lerp(Vector3D(0, 0, 0), delta, factor).Evaluate();
		auto moveF = Lerp(Vector3D(0, 0, 0), deltaF, factor).Evaluate();
		auto moveF2 = Lerp(Vector3D(0, 0.5, 0), deltaF2, factor).Evaluate();

		nc.body.first += move + Vector3D(0, d, 0);
		nc.foot[0].first += moveF;
		nc.foot[1].first += moveF2;

		auto m = DXMathTransform<double>::RotationY(-M_PI / 2 * i / phases);
		auto rot = ExponentialMap::FromMatrix(m);
		nc.body.second = rot;

		coords.push_back(make_pair(i * g_timeStep, nc));
	}

	return new SolutionVector(coords);
}

//------------------------------------------------------------------------------
void SolutionVector::Dump()
{
	FILE *f = nullptr;
	fopen_s(&f, "acclog.txt", "w");
	if (f == NULL)
	{
		return;
	}

	fprintf(f,
		"time\t"
		"b_p_x\ty\tz\t"		"b_pv_x\ty\tz\t"	"b_pa_x\ty\tz\t"
		"b_r_x\ty\tz\t"		"b_rv_x\ty\tz\t"	"b_ra_x\ty\tz\t"
		"l1_r_x\ty\tz\t"	"l1_rv_x\ty\tz\t"	"l1_ra_x\ty\tz\t"
		"l2_r_x\ty\tz\t"	"l2_rv_x\ty\tz\t"	"l2_ra_x\ty\tz\t"
		"\n");

	auto& dump = [&f](
		double t,
		const GeneralCoordinate& g,
		const GeneralCoordinate& gv,
		const GeneralCoordinate& ga)
	{
		fprintf(f,
			"%f\t"
			"%f\t%f\t%f\t"		"%f\t%f\t%f\t"		"%f\t%f\t%f\t"
			"%f\t%f\t%f\t"		"%f\t%f\t%f\t"		"%f\t%f\t%f\t"
			"%f\t%f\t%f\t"		"%f\t%f\t%f\t"		"%f\t%f\t%f\t"
			"%f\t%f\t%f\t"		"%f\t%f\t%f\t"		"%f\t%f\t%f\t"
			"\n",
			t,
			g.body.first.x, g.body.first.y, g.body.first.z,
			gv.body.first.x, gv.body.first.y, gv.body.first.z,
			ga.body.first.x, ga.body.first.y, ga.body.first.z,

			g.body.second.x, g.body.second.y, g.body.second.z,
			gv.body.second.x, gv.body.second.y, gv.body.second.z,
			ga.body.second.x, ga.body.second.y, ga.body.second.z,

			g.leg[0].rot1.x, g.leg[0].rot1.y, g.leg[0].rot1.z,
			gv.leg[0].rot1.x, gv.leg[0].rot1.y, gv.leg[0].rot1.z,
			ga.leg[0].rot1.x, ga.leg[0].rot1.y, ga.leg[0].rot1.z,

			g.leg[0].rot2.x, g.leg[0].rot2.y, g.leg[0].rot2.z,
			gv.leg[0].rot2.x, gv.leg[0].rot2.y, gv.leg[0].rot2.z,
			ga.leg[0].rot2.x, ga.leg[0].rot2.y, ga.leg[0].rot2.z
		);
	};

	double timeFrom = 0;
	double timeTo = splines[0].curve->GetMax();

	GeneralCoordinate zero;
	zero.Clear();

	const double derStep = 0.001;

	{
		double t = timeFrom - derStep;

		auto c = At(t);
		robot->Apply(c);
		auto g = robot->Current();

		dump(t, g, zero, zero);
	}

	//for (double t = timeFrom; t <= timeTo; t += derStep)
	double p = 1.573;
	for (double t = p - derStep; t <= p + derStep; t += derStep)
		//for (double t = 0.45; t <= 0.5010; t += derStep)
		//for (double t = 0.0; t <= 1.0; t += derStep)
	{
		auto cmm = At(t - derStep * 2);
		auto cm = At(t - derStep);
		auto c = At(t);
		auto cp = At(t + derStep);
		auto cpp = At(t + derStep * 2);

		robot->Apply(cm);
		auto gm = robot->Current();

		robot->Apply(c);
		auto g = robot->Current();

		robot->Apply(cp);
		auto gp = robot->Current();

		gm.MakeNear(g);
		gp.MakeNear(g);

		auto gv = (gp - gm) / (2 * derStep);

		// https://www.scss.tcd.ie/~dahyotr/CS7ET01/01112007.pdf
		auto ga = (gp - (g * 2.0) + gm) / (derStep * derStep);

		dump(t, g, gv, ga);
	}

	{
		double t = timeTo + derStep;

		auto c = At(t);
		robot->Apply(c);
		auto g = robot->Current();

		dump(t, g, zero, zero);
	}

	fclose(f);
}
