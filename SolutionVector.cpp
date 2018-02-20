#include "pch.h"
#include "SolutionVector.h"

#include <stdio.h>
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

		double d = 0;
		d = i % 2 == 0 ? 0.05 : -0.05;

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
		splines.body.Append(it->first, coord.body);
		splines.foot[0].Append(it->first, coord.foot[0]);
		splines.foot[1].Append(it->first, coord.foot[1]);
	}

	splines.body.Update();
	splines.foot[0].Update();
	splines.foot[1].Update();
}

//------------------------------------------------------------------------------
SolutionCoordinate SolutionVector::At(double t)
{
	SolutionCoordinate c;

	c.body = splines.body.curve->At(t);
	c.foot[0] = splines.foot[0].curve->At(t);
	c.foot[1] = splines.foot[1].curve->At(t);

	return c;
}

////------------------------------------------------------------------------------
//SolutionCoordinate SolutionVector::AtByTime(double time)
//{
//	return AtByFactor(time / g_timeStep);
//}

//------------------------------------------------------------------------------
void SolutionVector::UpdateGenericCoordinates()
{
	return;

	FILE *f = nullptr;
	fopen_s(&f, "acclog.txt", "w");
	if (f == NULL)
	{
		return;
	}

	auto& dump = [&f](
		double t, 
		const GeneralCoordinate& g, 
		const GeneralCoordinate& gv,
		const GeneralCoordinate& ga)
	{
		fprintf(f,
			"%f\t"
			"%f\t%f\t%f\t"
			"%f\t%f\t%f\t"
			"%f\t%f\t%f\t"
			"\n",
			t,
			g.body.first.x,
			g.body.first.y,
			g.body.first.z,
			gv.body.first.x,
			gv.body.first.y,
			gv.body.first.z,
			ga.body.first.x,
			ga.body.first.y,
			ga.body.first.z);
	};

	fprintf(f, 
		"time\t"
		"b_p_x\ty\tz\t"
		"b_pv_x\ty\tz\t"
		"b_pa_x\ty\tz\t"
		"\n");

	gAcc.clear();

	auto m = splines.body.curve->GetMax();

	double timeFrom = 0;
	double timeTo = m * g_timeStep;

	GeneralCoordinate zero;
	zero.Clear();

	{
		double t = timeFrom - g_derStep;

		auto c = At(t);
		robot->Apply(c);
		auto g = robot->Current();

		dump(t, g, zero, zero);
	}

	for (double t = timeFrom; t <= timeTo; t += g_derStep)
	//for (double t = 0.45; t <= 0.5010; t += g_derStep)
	//for (double t = 0.0; t <= 1.0; t += g_derStep)
	{
		auto cm = At(t - g_derStep);
		auto c = At(t);
		auto cp = At(t + g_derStep);

		robot->Apply(cm);
		auto gm = robot->Current();

		robot->Apply(c);
		auto g = robot->Current();

		robot->Apply(cp);
		auto gp = robot->Current();

		auto gv = (gp - gm) / (2 * g_derStep);

		// https://www.scss.tcd.ie/~dahyotr/CS7ET01/01112007.pdf
		auto ga = (gp - (g * 2.0) + gm) / (g_derStep * g_derStep);

		dump(t, g, gv, ga);
	}

	{
		double t = timeFrom + g_derStep;

		auto c = At(t);
		robot->Apply(c);
		auto g = robot->Current();

		dump(t, g, zero, zero);
	}

	fclose(f);
}
