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

	AnimationGeneration(IRobot* robot) : robot(robot)
	{
		solution = SolutionVector::BuildTest(robot->CurrentSC());
	}

	void UpdateSpline()
	{
		SplineSource body, foot[2];

		for (auto it = solution.coords.begin(); 
			it != solution.coords.end(); ++it)
		{
			auto coord = it->second;
			
			body.pos.push_back(coord.bodyPos);
			body.rot.push_back(coord.bodyRot);

			foot[0].pos.push_back(coord.footPos[0]);
			foot[0].rot.push_back(coord.footRot[0]);

			foot[1].pos.push_back(coord.footPos[1]);
			foot[1].rot.push_back(coord.footRot[1]);
		}

		splines.body.reset(new SplineDiagnostic(body.pos, body.rot));
		splines.foot[0].reset(new SplineDiagnostic(foot[0].pos, foot[0].rot));
		splines.foot[1].reset(new SplineDiagnostic(foot[1].pos, foot[1].rot));
	}

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

		splines.body->Enqueue(buffer, factor);
		splines.foot[0]->Enqueue(buffer, factor);
		splines.foot[1]->Enqueue(buffer, factor);

		auto coord = At(factor);

		robot->Apply(coord);
	}

	SolutionCoordinate At(double factor)
	{
		SolutionCoordinate coord;

		{
			auto b = splines.body->spline->At(factor);
			coord.bodyPos = b.first;
			coord.bodyRot = b.second;
		}

		{
			auto b = splines.foot[0]->spline->At(factor);
			coord.footPos[0] = b.first;
			coord.footRot[0] = b.second;
		}

		{
			auto b = splines.foot[1]->spline->At(factor);
			coord.footPos[1] = b.first;
			coord.footRot[1] = b.second;
		}

		return coord;
	}

	SolutionVector solution;

	struct SplineSource
	{
		vector<Vector3D> pos;
		vector<Vector3D> rot;
	};

	struct Splines
	{
		unique_ptr<SplineDiagnostic> body;
		unique_ptr<SplineDiagnostic> foot[2];
	};

	Splines splines;
};

IAnimationGeneration* IAnimationGeneration::Create(IRobot* robot)
{
	return new AnimationGeneration(robot);
}

