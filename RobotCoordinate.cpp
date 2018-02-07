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
	auto Set = [&](const string& name, const Matrix4D& m)
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

	auto bodyQuat = ExponentialMap::ToQuaternion(coord.bodyRot);

	auto bodyM = DXMathTransform<double>::MatrixRotationQuaternion(bodyQuat);
	FrameHelper::SetTranslation(bodyM, coord.bodyPos);

	Set("Body", bodyM);
}

