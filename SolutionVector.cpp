#include "pch.h"
#include "SolutionVector.h"

#include <WindowsUtility.h>

#include "Robot.h"

using namespace std;
using namespace Core;

const double g_timeStep = 0.5;
const double g_derStep = 0.001;

//------------------------------------------------------------------------------
SolutionVector::SolutionVector()
{
	robot.reset(IRobot::Create());
}

//------------------------------------------------------------------------------
double SolutionVector::Timestep()
{
	return g_timeStep;
}

//------------------------------------------------------------------------------
SolutionVector* SolutionVector::BuildTest(const SolutionCoordinate& init)
{
	auto v = new SolutionVector;

	auto delta = Vector3D(2, 0, 0);
	auto deltaF = Vector3D(1.5, 0.5f, 0);
	auto deltaF2 = Vector3D(3, 0, 0);

	int phases = 8;

	for (int i = 0; i <= phases; ++i)
	{
		double factor = (double)i / phases;

		SolutionCoordinate nc = init;

		double d = -0.05;
		if (i % 2 == 0)
		{
			d = 0.05;
		}

		auto move = Lerp(Vector3D(0, 0, 0), delta, factor).Evaluate();
		auto moveF = Lerp(Vector3D(0, 0, 0), deltaF, factor).Evaluate();
		auto moveF2 = Lerp(Vector3D(0, 0.5, 0), deltaF2, factor).Evaluate();

		nc.body.first += move + Vector3D(0, d, 0);
		nc.foot[0].first += moveF;
		nc.foot[1].first += moveF2;

		v->coords.push_back(make_pair(i * g_timeStep, nc));
	}

	v->Update();

	return v;
}

//------------------------------------------------------------------------------
void SolutionVector::Update()
{
	UpdateSpline();

	UpdateGenericCoordinates();
}

//------------------------------------------------------------------------------
void SolutionVector::UpdateSpline()
{
	for (auto it = coords.begin(); it != coords.end(); ++it)
	{
		auto coord = it->second;
		splines.body.Append(coord.body);
		splines.foot[0].Append(coord.foot[0]);
		splines.foot[1].Append(coord.foot[1]);
	}

	splines.body.Update();
	splines.foot[0].Update();
	splines.foot[1].Update();
}

//------------------------------------------------------------------------------
SolutionCoordinate SolutionVector::AtByFactor(double f)
{
	SolutionCoordinate c;

	c.body = splines.body.curve->At(f);
	c.foot[0] = splines.foot[0].curve->At(f);
	c.foot[1] = splines.foot[1].curve->At(f);

	return c;
}

//------------------------------------------------------------------------------
SolutionCoordinate SolutionVector::AtByTime(double time)
{
	return AtByFactor(time / g_timeStep);
}

//------------------------------------------------------------------------------
void SolutionVector::UpdateGenericCoordinates()
{
	gAcc.clear();

	auto m = splines.body.curve->GetMax();

	//for (double t = 0; t < m * g_timeStep; t += g_derStep)
	for (double t = 0.45; t <= 0.5010; t += g_derStep)
	{
		auto cm = AtByTime(t - g_derStep);
		auto c = AtByTime(t);
		auto cp = AtByTime(t + g_derStep);

		robot->Apply(cm);
		auto gm = robot->Current();

		robot->Apply(c);
		auto g = robot->Current();

		robot->Apply(cp);
		auto gp = robot->Current();

		// https://www.scss.tcd.ie/~dahyotr/CS7ET01/01112007.pdf
		auto ga = (gp - (g * 2.0) + gm) / (g_derStep * g_derStep);

		WindowsUtility::Debug(L"time %.4f ", t);
		gm.Dump();
		g.Dump();
		gp.Dump();
		ga.Dump();
		WindowsUtility::Debug(L"\n");
	}
}
