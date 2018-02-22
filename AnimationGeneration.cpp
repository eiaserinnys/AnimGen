#include "pch.h"
#include "AnimationGeneration.h"

#include <timeapi.h>

#include "Robot.h"
#include "SolutionVector.h"
#include "Solver.h"

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

	unique_ptr<ISolver> solver;

	//--------------------------------------------------------------------------
	AnimationGeneration(IRobot* robot) : robot(robot)
	{
		SolutionCoordinate begin = robot->CurrentSC();
		
		SolutionCoordinate dest = begin;
		dest.body.first += Vector3D(2.5, 0, 0);
		dest.foot[0].first += Vector3D(2.5, 0, 0);
		dest.foot[1].first += Vector3D(2.5, 0, 0);

		sol.reset(ISolutionVector::Create(begin, 8));

		solver.reset(ISolver::Create(begin, dest, 8));

		// Å×½ºÆ®!
		solver->Begin();
	}

	//--------------------------------------------------------------------------
	void UpdateSpline()
	{
		const int m = 20;

		splines.body.Sample(sol->GetCurve(0), m);
		splines.foot[0].Sample(sol->GetCurve(1), m);
		splines.foot[1].Sample(sol->GetCurve(2), m);
	}

	//--------------------------------------------------------------------------
	double CurrentTime()
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

		double t = sol->GetLastPhaseTime();
		int total = (int)(t * 1000);

		elapsed = elapsed % total;

		return ((double)elapsed / total) * t;
	}

	//--------------------------------------------------------------------------
	void Enqueue(LineBuffer* buffer)
	{
		auto t = CurrentTime();

		splines.body.Enqueue(sol->GetCurve(0), buffer, t);
		splines.foot[0].Enqueue(sol->GetCurve(1), buffer, t);
		splines.foot[1].Enqueue(sol->GetCurve(2), buffer, t);

		auto coord = sol->At(t);

		//robot->Apply(coord);
	}

	//--------------------------------------------------------------------------
	unique_ptr<ISolutionVector> sol;

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

