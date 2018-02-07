#include "pch.h"
#include "RobotCoordinate.h"

#include "ExponentialMap.h"
#include "DXMathTransform.h"
#include "FrameHelper.h"
#include "RobotImplementation.h"

using namespace std;
using namespace Core;

//--------------------------------------------------------------------------
// 일단 몸, 다리, 발만 계산한다
IRobot::GeneralCoordinate 
	RobotCoordinate::ToGeneralCoordinate(Robot* robot)
{
	IRobot::GeneralCoordinate coord;

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
void RobotCoordinate::FromGeneralCoordinate(
	Robot* robot,
	const IRobot::GeneralCoordinate& coord)
{
	auto SetWorld = [&](const string& name, const Matrix4D& m)
	{
		auto body = robot->Find(name);

		if (body != nullptr)
		{
			assert(m.AssertEqual(body->WorldTx()));
			body->SetWorldTx(m);
		}
		else
		{
			assert(!"body not found");
		}
	};

	auto SetLocal = [&](const string& name, const Matrix4D& m)
	{
		auto body = robot->Find(name);

		if (body != nullptr)
		{
			assert(m.AssertEqual(body->LocalTx()));
			body->SetLocalTx(m);
		}
		else
		{
			assert(!"body not found");
		}
	};

	{
		auto bodyM = ExponentialMap::ToMatrix(coord.bodyRot);
		FrameHelper::SetTranslation(bodyM, coord.bodyPos);
		SetWorld("Body", bodyM);
	}

	SetLocal("LLeg1", ExponentialMap::ToMatrix(coord.leg[0].rot1));
	SetLocal("RLeg1", ExponentialMap::ToMatrix(coord.leg[1].rot1));

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[0].rot2);
		FrameHelper::SetTranslation(m, Vector3D(coord.leg[0].len1, 0, 0));
		SetLocal("LLeg2", m);
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[0].footRot);
		FrameHelper::SetTranslation(m, Vector3D(0, - coord.leg[0].len2, 0));
		SetLocal("LFoot", m);
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[1].rot2);
		FrameHelper::SetTranslation(m, Vector3D(coord.leg[1].len1, 0, 0));
		SetLocal("RLeg2", m);
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[1].footRot);
		FrameHelper::SetTranslation(m, Vector3D(0, -coord.leg[1].len2, 0));
		SetLocal("RFoot", m);
	}
}

