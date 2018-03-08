#include "pch.h"
#include "RobotCoordinate.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "ExponentialMap.h"
#include "DXMathTransform.h"
#include "FrameHelper.h"
#include "RobotImplementation.h"

#include "GeneralizedCoordinate.h"
#include "SolutionCoordinate.h"

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
bool SetLocal(Robot* robot, const string& name, const Matrix4D& m, bool validate)
{
	bool result = true;
	auto body = robot->Find(name);

	if (body != nullptr)
	{
		if (validate)
		{
			if (IsDebuggerPresent())
			{
				result = m.AssertEqual(body->LocalTx());
				if (!result)
				{
					WindowsUtility::Debug(
						L"Local transform is different at bone (%S)\n",
						name.c_str());
				}
			}
			else
			{
				assert(m.AssertEqual(body->LocalTx()));
			}
		}
		body->SetLocalTx(m);
	}
	else
	{
		assert(!"body not found");
	}
	return result;
};


//--------------------------------------------------------------------------
// 일단 몸, 다리, 발만 계산한다
GeneralizedCoordinate 
	RobotCoordinate::ToGeneralizedCoordinate(Robot* robot) const
{
	GeneralizedCoordinate coord;

	coord.body.position = robot->GetWorldPosition("Body");
	coord.body.rotation = robot->GetLocalRotation("Body");

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
bool RobotCoordinate::SetTransform(
	Robot* robot,
	const GeneralizedCoordinate& coord, 
	bool validate) const
{
	bool result = true;

	{
		auto bodyM = ExponentialMap::ToMatrix(coord.body.rotation);
		FrameHelper::SetTranslation(bodyM, coord.body.position);
		SetWorld(robot, "Body", bodyM, validate);
	}

	result = SetLocal(robot, "LLeg1", ExponentialMap::ToMatrix(coord.leg[0].rot1), validate) && result;
	result = SetLocal(robot, "RLeg1", ExponentialMap::ToMatrix(coord.leg[1].rot1), validate) && result;

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[0].rot2);
		FrameHelper::SetTranslation(m, Vector3D(coord.leg[0].len1, 0, 0));
		result = SetLocal(robot, "LLeg2", m, validate) && result;
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[0].footRot);
		FrameHelper::SetTranslation(m, Vector3D(0, - coord.leg[0].len2, 0));
		result = SetLocal(robot, "LFoot", m, validate) && result;
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[1].rot2);
		FrameHelper::SetTranslation(m, Vector3D(coord.leg[1].len1, 0, 0));
		result = SetLocal(robot, "RLeg2", m, validate) && result;
	}

	{
		auto m = ExponentialMap::ToMatrix(coord.leg[1].footRot);
		FrameHelper::SetTranslation(m, Vector3D(0, -coord.leg[1].len2, 0));
		result = SetLocal(robot, "RFoot", m, validate) && result;
	}

	return result;
}

//--------------------------------------------------------------------------
SolutionCoordinate RobotCoordinate::ToSolutionCoordinate(Robot* robot) const
{
	SolutionCoordinate coord;

	coord.body.position = robot->GetWorldPosition("Body");
	coord.body.rotation = robot->GetLocalRotation("Body");

	coord.foot[0].position = robot->GetWorldPosition("LFoot");
	coord.foot[0].rotation = ExponentialMap::FromMatrix(robot->GetWorldTransform("LFoot"));

	coord.foot[1].position = robot->GetWorldPosition("RFoot");
	coord.foot[1].rotation = ExponentialMap::FromMatrix(robot->GetWorldTransform("RFoot"));

	return coord;
}

//--------------------------------------------------------------------------
void RobotCoordinate::SetTransform(
	Robot* robot,
	const SolutionCoordinate& coord, 
	bool validate) const
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
		robot->UpdateWorldTransform();

		for (int i = 0; i < COUNT_OF(names); ++i)
		{
			preserved.push_back(robot->Find(names[i])->WorldTx());
		}
	}

	bool retry = false;

	do
	{
		retry = false;

		{
			auto bodyM = ExponentialMap::ToMatrix(coord.body.rotation);
			FrameHelper::SetTranslation(bodyM, coord.body.position);
			SetWorld(robot, "Body", bodyM, false);
		}

		robot->SetFootTransform(true, coord.foot[0].position, coord.foot[0].rotation);
		robot->SetFootTransform(false, coord.foot[1].position, coord.foot[1].rotation);

		if (validate)
		{
			bool valid = true;

			robot->UpdateWorldTransform();

			for (int i = 0; i < COUNT_OF(names); ++i)
			{
				auto newWorldTx = robot->Find(names[i])->WorldTx();

				if (IsDebuggerPresent())
				{
					bool check = preserved[i].AssertEqual(newWorldTx);

					if (!check)
					{
						WindowsUtility::Debug(
							L"World Transform at Bone(%d,%S) is different\n",
							i, names[i]);

						auto a = preserved[i];
						auto b = newWorldTx;

						WindowsUtility::Debug(
							L"(%.3f, %.3f, %.3f, %.3f) "
							"(%.3f, %.3f, %.3f, %.3f) "
							"(%.3f, %.3f, %.3f, %.3f) "
							"(%.3f, %.3f, %.3f, %.3f)\n",
							a.e[0 * 4 + 0], a.e[0 * 4 + 1], a.e[0 * 4 + 2], a.e[0 * 4 + 3],
							a.e[1 * 4 + 0], a.e[1 * 4 + 1], a.e[1 * 4 + 2], a.e[1 * 4 + 3],
							a.e[2 * 4 + 0], a.e[2 * 4 + 1], a.e[2 * 4 + 2], a.e[2 * 4 + 3],
							a.e[3 * 4 + 0], a.e[3 * 4 + 1], a.e[3 * 4 + 2], a.e[3 * 4 + 3]);

						WindowsUtility::Debug(
							L"(%.3f, %.3f, %.3f, %.3f) "
							"(%.3f, %.3f, %.3f, %.3f) "
							"(%.3f, %.3f, %.3f, %.3f) "
							"(%.3f, %.3f, %.3f, %.3f)\n",
							b.e[0 * 4 + 0], b.e[0 * 4 + 1], b.e[0 * 4 + 2], b.e[0 * 4 + 3],
							b.e[1 * 4 + 0], b.e[1 * 4 + 1], b.e[1 * 4 + 2], b.e[1 * 4 + 3],
							b.e[2 * 4 + 0], b.e[2 * 4 + 1], b.e[2 * 4 + 2], b.e[2 * 4 + 3],
							b.e[3 * 4 + 0], b.e[3 * 4 + 1], b.e[3 * 4 + 2], b.e[3 * 4 + 3]);
					}

					valid = valid && check;
				}
				else
				{
					assert(preserved[i].AssertEqual(newWorldTx));
				}
			}

			if (IsDebuggerPresent() && !valid)
			{
				retry = true;
				DebugBreak();
			}
		}
	} 
	while (retry);
}

//--------------------------------------------------------------------------
void SolutionCoordinate::Dump() const
{
	WindowsUtility::Debug(
		L"B(%.10f,%.10f,%.10f), (%.10f,%.10f,%.10f) "
		"LF(%.10f,%.10f,%.10f), (%.10f,%.10f,%.10f) "
		"RF(%.10f,%.10f,%.10f), (%.10f,%.10f,%.10f)\n",
		body.position.x, body.position.y, body.position.z,
		body.rotation.x, body.rotation.y, body.rotation.z,
		foot[0].position.x, foot[0].position.y, foot[0].position.z,
		foot[0].rotation.x, foot[0].rotation.y, foot[0].rotation.z,
		foot[1].position.x, foot[1].position.y, foot[1].position.z,
		foot[1].rotation.x, foot[1].rotation.y, foot[1].rotation.z);
}