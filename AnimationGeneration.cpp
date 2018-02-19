#include "pch.h"
#include "AnimationGeneration.h"

#include <timeapi.h>

#include "Robot.h"
#include "SolutionVector.h"
#include "SplineDiagnostic.h"
#include "LineBuffer.h"

using namespace std;
using namespace Core;

//------------------------------------------------------------------------------
class AnimationGeneration : public IAnimationGeneration {
public:
	DWORD lastTime = 0;
	DWORD elapsed = 0;

	IRobot* robot;

	//--------------------------------------------------------------------------
	AnimationGeneration(IRobot* robot) : robot(robot)
	{
		sol.reset(SolutionVector::BuildTest(robot->CurrentSC()));
	}

	//--------------------------------------------------------------------------
	void UpdateSpline()
	{
		const int m = 20;

		splines.body.Sample(sol->splines.body.curve.get(), m);
		splines.foot[0].Sample(sol->splines.foot[0].curve.get(), m);
		splines.foot[1].Sample(sol->splines.foot[1].curve.get(), m);
	}

	//--------------------------------------------------------------------------
	double CurrentFactor()
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

		int points = sol->coords.size();
		int total = (int)((points - 1) * sol->Timestep() * 1000);

		elapsed = elapsed % total;

		return ((double)elapsed / total) * (points - 1);
	}

	//--------------------------------------------------------------------------
	void Enqueue(LineBuffer* buffer)
	{
		auto factor = CurrentFactor();

		splines.body.Enqueue(sol->splines.body.curve.get(), buffer, factor);
		splines.foot[0].Enqueue(sol->splines.foot[0].curve.get(), buffer, factor);
		splines.foot[1].Enqueue(sol->splines.foot[1].curve.get(), buffer, factor);

		auto coord = sol->AtByFactor(factor);

		robot->Apply(coord);
	}

	//--------------------------------------------------------------------------
	unique_ptr<SolutionVector> sol;

	struct Diagnostic
	{
		SplineDiagnostic body;
		SplineDiagnostic foot[2];
	};

	Diagnostic splines;
};

//------------------------------------------------------------------------------
IAnimationGeneration* IAnimationGeneration::Create(IRobot* robot)
{
	return new AnimationGeneration(robot);
}

