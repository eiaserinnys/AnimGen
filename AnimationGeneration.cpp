#include "pch.h"
#include "AnimationGeneration.h"

#include "Robot.h"
#include "SolutionVector.h"
#include "SplineDiagnostic.h"
#include "LineBuffer.h"

using namespace std;
using namespace Core;

class AnimationGeneration : public IAnimationGeneration {
public:
	AnimationGeneration(IRobot* robot)
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
		splines.body->Enqueue(buffer);
		splines.foot[0]->Enqueue(buffer);
		splines.foot[1]->Enqueue(buffer);
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

