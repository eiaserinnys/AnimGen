#include "pch.h"
#include "AnimationGeneration.h"

#include <timeapi.h>
#include <Utility.h>

#include "Robot.h"
#include "SolutionVector.h"
#include "Solver.h"
#include "SolverLog.h"

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

	struct Diagnostic
	{
		SplineDiagnostic joint[3];
	};

	Diagnostic splines;

	//--------------------------------------------------------------------------
	AnimationGeneration(IRobot* robot) : robot(robot)
	{
		SolutionCoordinate begin = robot->CurrentSC();
		
		SolutionCoordinate dest = begin;

		for (int i = 0; i < COUNT_OF(dest.joint); ++i)
		{
			dest.joint[i].position += Vector3D(2.5, 0, 0);
		}

		solver.reset(ISolver::Create(begin, dest, 2));
		//solver.reset(ISolver::Create(begin, dest, 1));
	}

	//--------------------------------------------------------------------------
	void Begin()
	{
		auto log = ISolverLog::Create();

		log->Open(
			"log_residual.txt", 
			"log_move.txt", 
			"log_jacobian.txt",
			nullptr);

		solver->Begin(log, true);
	}

	//--------------------------------------------------------------------------
	ISolver::Result::Value Step()
	{
		return solver->SolveStep();
	}

	//--------------------------------------------------------------------------
	void End()
	{
		solver->End();
	}

	//--------------------------------------------------------------------------
	void UpdateSpline()
	{
		const int m = 20;

		auto sol = solver->Solution();
		for (int i = 0; i < sol->GetSplineCount(); ++i)
		{
			splines.joint[i].Sample(sol->GetSpline(i), m);
		}
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

		auto sol = solver->Solution();

		double t = sol->GetLastPhaseTime();
		int total = (int)(t * 1000);

		elapsed = elapsed % total;

		return ((double)elapsed / total) * t;
	}

	//--------------------------------------------------------------------------
	void Enqueue(LineBuffer* buffer)
	{
		auto t = CurrentTime();

		auto sol = solver->Solution();
		for (int i = 0; i < sol->GetSplineCount(); ++i)
		{
			splines.joint[i].Enqueue(sol->GetSpline(i), buffer, t);
		}

		auto coord = sol->At(t);

		//robot->Apply(coord);
	}
};

//------------------------------------------------------------------------------
IAnimationGeneration* IAnimationGeneration::Create(IRobot* robot)
{
	return new AnimationGeneration(robot);
}

