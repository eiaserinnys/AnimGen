#include "pch.h"
#include "RobotCoordinate.h"

#include <Utility.h>

#include "ExponentialMap.h"
#include "DXMathTransform.h"
#include "FrameHelper.h"
#include "RobotImplementation.h"

using namespace std;
using namespace Core;

//--------------------------------------------------------------------------
void SetWorld(Robot* robot, const string& name, const Matrix4D& m, bool validate)
{
	auto body = robot->Find(name);

	if (body != nullptr)
	{
		if (validate)
		{
			assert(m.AssertEqual(body->WorldTx()));
		}
		body->SetWorldTx(m);
	}
	else
	{
		assert(!"body not found");
	}
};

//--------------------------------------------------------------------------
void SetLocal(Robot* robot, const string& name, const Matrix4D& m, bool validate)
{
	auto body = robot->Find(name);

	if (body != nullptr)
	{
		if (validate)
		{
			assert(m.AssertEqual(body->LocalTx()));
		}
		body->SetLocalTx(m);
	}
	else
	{
		assert(!"body not found");
	}
};

//--------------------------------------------------------------------------
// 일단 몸, 다리, 발만 계산한다
GeneralCoordinate 
	RobotCoordinate::ToGeneralCoordinate(Robot* robot)
{
	GeneralCoordinate coord;

	coord.bodyPos = robot->GetWorldPosition("Body");
	coord.bodyRot = robot->GetLocalRotation("Body");

	{
		coord.leg[0].rot1 = robot->GetLocalRotation("LLeg1");

		{
			auto p0 = robot->GetWorldPosition("LLeg1");
			auto p1 = robot->GetWorldPosition("LLeg2");
			coord.leg[0].len1 = Distance(p0, p1) - robot->legLen.x;
		}

		coord.leg[0].rot2 = robot->GetLocalRotation("LLeg2");

		{
			auto p0 = robot->GetWorldPosition("LLeg2");
			auto p1 = robot->GetWorldPosition("LFoot");
			coord.leg[0].len2 = Distance(p0, p1) - robot->legLen.y;
		}

		coord.leg[0].footRot = robot->GetLocalRotation("LFoot");
	}

	{
		coord.leg[1].rot1 = robot->GetLocalRotation("RLeg1");

		{
			auto p0 = robot->GetWorldPosition("RLeg1");
			auto p1 = robot->GetWorldPosition("RLeg2");
			coord.leg[1].len1 = Distance(p0, p1) - robot->legLen.x;
		}

		coord.leg[1].rot2 = robot->GetLocalRotation("RLeg2");

		{
			auto p0 = robot->GetWorldPosition("RLeg2");
			auto p1 = robot->GetWorldPosition("RFoot");
			coord.leg[1].len2 = Distance(p0, p1) - robot->legLen.y;
		}

		coord.leg[1].footRot = robot->GetLocalRotation("RFoot");
	}

	return coord;
}

//--------------------------------------------------------------------------
void RobotCoordinate::SetTransform(
	Robot* robot,
	const GeneralCoordinate& coord, 
	bool validate)
{
	{
		auto bodyM = ExponentialMap::ToMatrix(coord.bodyRot);
		FrameHelper::SetTranslation(bodyM, coord.bodyPos);
		SetWorld(robot, "Body", bodyM, validate);
	}

	SetLocal(robot, "LLeg1", ExponentialMap::ToMatrix(coord.leg[0].rot1), validate);
	SetLocal(robot, "RLeg1", ExponentialMap::ToMatrix(coord.leg[1].rot1), validate);

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[0].rot2);
		FrameHelper::SetTranslation(m, Vector3D(coord.leg[0].len1, 0, 0));
		SetLocal(robot, "LLeg2", m, validate);
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[0].footRot);
		FrameHelper::SetTranslation(m, Vector3D(0, - coord.leg[0].len2, 0));
		SetLocal(robot, "LFoot", m, validate);
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[1].rot2);
		FrameHelper::SetTranslation(m, Vector3D(coord.leg[1].len1, 0, 0));
		SetLocal(robot, "RLeg2", m, validate);
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[1].footRot);
		FrameHelper::SetTranslation(m, Vector3D(0, -coord.leg[1].len2, 0));
		SetLocal(robot, "RFoot", m, validate);
	}
}

//--------------------------------------------------------------------------
SolutionCoordinate RobotCoordinate::ToSolutionCoordinate(Robot* robot)
{
	SolutionCoordinate coord;

	coord.bodyPos = robot->GetWorldPosition("Body");
	coord.bodyRot = robot->GetLocalRotation("Body");

	coord.footPos[0] = robot->GetWorldPosition("LFoot");
	coord.footRot[0] = ExponentialMap::FromMatrix(robot->GetWorldTransform("LFoot"));

	coord.footPos[1] = robot->GetWorldPosition("RFoot");
	coord.footRot[1] = ExponentialMap::FromMatrix(robot->GetWorldTransform("RFoot"));

	return coord;
}

//--------------------------------------------------------------------------
void RobotCoordinate::SetTransform(
	Robot* robot,
	const SolutionCoordinate& coord, 
	bool validate)
{
	static const char* names[] =
	{
		"Body", 
		"LLeg1", "LLeg2","LFoot",
		"RLeg1", "RLeg2","RFoot",
	};

	vector<Matrix4D> preserved;
	if (validate)
	{
		for (int i = 0; i < COUNT_OF(names); ++i)
		{
			preserved.push_back(robot->Find(names[i])->WorldTx());
		}
	}

	{
		auto bodyM = ExponentialMap::ToMatrix(coord.bodyRot);
		FrameHelper::SetTranslation(bodyM, coord.bodyPos);
		SetWorld(robot, "Body", bodyM, false);
	}

	robot->SetFootTransform(true, coord.footPos[0], coord.footRot[0]);
	robot->SetFootTransform(false, coord.footPos[1], coord.footRot[1]);

	if (validate)
	{
		for (int i = 0; i < COUNT_OF(names); ++i)
		{
			auto newWorldTx = robot->Find(names[i])->WorldTx();
			assert(preserved[i].AssertEqual(newWorldTx));
		}
	}
}
