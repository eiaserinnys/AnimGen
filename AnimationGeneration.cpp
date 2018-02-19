#include "pch.h"
#include "AnimationGeneration.h"

#include <timeapi.h>

#include "Robot.h"
#include "SolutionVector.h"
#include "SplineDiagnostic.h"
#include "LineBuffer.h"

using namespace std;
using namespace Core;

class AnimationGeneration : public IAnimationGeneration {
public:
	DWORD lastTime = 0;
	DWORD elapsed = 0;

	IRobot* robot;

	//--------------------------------------------------------------------------
	AnimationGeneration(IRobot* robot) : robot(robot)
	{
		solution = SolutionVector::BuildTest(robot->CurrentSC());
	}

	//--------------------------------------------------------------------------
	void UpdateSpline()
	{
		const int m = 20;

		splines.body.Sample(solution.splines.body.curve.get(), m);
		splines.foot[0].Sample(solution.splines.foot[0].curve.get(), m);
		splines.foot[1].Sample(solution.splines.foot[1].curve.get(), m);
	}

	//--------------------------------------------------------------------------
	void Enqueue(LineBuffer* buffer)
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

		int points = solution .coords.size();
		int total = (int)((points - 1) * 0.5 * 1000);

		elapsed = elapsed % total;

		double factor = ((double)elapsed / total) * (points - 1);

		splines.body.Enqueue(solution.splines.body.curve.get(), buffer, factor);
		splines.foot[0].Enqueue(solution.splines.foot[0].curve.get(), buffer, factor);
		splines.foot[1].Enqueue(solution.splines.foot[1].curve.get(), buffer, factor);

		auto coord = At(factor);

		robot->Apply(coord);
	}

	//--------------------------------------------------------------------------
	SolutionCoordinate At(double factor)
	{
		SolutionCoordinate coord;

		{
			auto b = solution.splines.body.curve->At(factor);
			coord.bodyPos = b.first;
			coord.bodyRot = b.second;
		}

		{
			auto b = solution.splines.foot[0].curve->At(factor);
			coord.footPos[0] = b.first;
			coord.footRot[0] = b.second;
		}

		{
			auto b = solution.splines.foot[1].curve->At(factor);
			coord.footPos[1] = b.first;
			coord.footRot[1] = b.second;
		}

		return coord;
	}

	//--------------------------------------------------------------------------
	SolutionVector solution;

	struct Splines
	{
		SplineDiagnostic body;
		SplineDiagnostic foot[2];
	};

	Splines splines;
};

IAnimationGeneration* IAnimationGeneration::Create(IRobot* robot)
{
	return new AnimationGeneration(robot);
}

