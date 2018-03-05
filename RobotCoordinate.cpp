#include "pch.h"
#include "RobotCoordinate.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "ExponentialMap.h"
#include "DXMathTransform.h"
#include "FrameHelper.h"
#include "RobotImplementation.h"

using namespace std;
using namespace Core;

bool operator == (const Vector3D& lhs, const Vector3D& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y &&lhs.z == rhs.z;
}

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
GeneralCoordinate::Leg operator + (
	const GeneralCoordinate::Leg& lhs, 
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 + rhs.rot1;
	ret.len1 = lhs.len1 + rhs.len1;
	ret.rot2 = lhs.rot2 + rhs.rot2;
	ret.len2 = lhs.len2 + rhs.len2;
	ret.footRot = lhs.footRot + rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator - (
	const GeneralCoordinate::Leg& lhs,
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 - rhs.rot1;
	ret.len1 = lhs.len1 - rhs.len1;
	ret.rot2 = lhs.rot2 - rhs.rot2;
	ret.len2 = lhs.len2 - rhs.len2;
	ret.footRot = lhs.footRot - rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator * (
	const GeneralCoordinate::Leg& lhs,
	double rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 * rhs;
	ret.len1 = lhs.len1 * rhs;
	ret.rot2 = lhs.rot2 * rhs;
	ret.len2 = lhs.len2 * rhs;
	ret.footRot = lhs.footRot * rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator * (
	double lhs, 
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs * rhs.rot1;
	ret.len1 = lhs * rhs.len1;
	ret.rot2 = lhs * rhs.rot2;
	ret.len2 = lhs * rhs.len2;
	ret.footRot = lhs * rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator * (
	const GeneralCoordinate::Leg& lhs,
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 * rhs.rot1;
	ret.len1 = lhs.len1 * rhs.len1;
	ret.rot2 = lhs.rot2 * rhs.rot2;
	ret.len2 = lhs.len2 * rhs.len2;
	ret.footRot = lhs.footRot * rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator / (
	const GeneralCoordinate::Leg& lhs,
	double rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 / rhs;
	ret.len1 = lhs.len1 / rhs;
	ret.rot2 = lhs.rot2 / rhs;
	ret.len2 = lhs.len2 / rhs;
	ret.footRot = lhs.footRot / rhs;

	return ret;
}

//--------------------------------------------------------------------------
bool operator == (
	const GeneralCoordinate::Leg& lhs,
	const GeneralCoordinate::Leg& rhs)
{
	return 
		lhs.rot1 == rhs.rot1 &&
		lhs.len1 == rhs.len1 &&
		lhs.rot2 == rhs.rot2 &&
		lhs.len2 == rhs.len2 &&
		lhs.footRot == rhs.footRot;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator - (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first - rhs.body.first;
	ret.body.second = lhs.body.second - rhs.body.second;
	ret.leg[0] = lhs.leg[0] - rhs.leg[0];
	ret.leg[1] = lhs.leg[1] - rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator + (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first + rhs.body.first;
	ret.body.second = lhs.body.second + rhs.body.second;
	ret.leg[0] = lhs.leg[0] + rhs.leg[0];
	ret.leg[1] = lhs.leg[1] + rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator / (const GeneralCoordinate& lhs, double rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first / rhs;
	ret.body.second = lhs.body.second / rhs;
	ret.leg[0] = lhs.leg[0] / rhs;
	ret.leg[1] = lhs.leg[1] / rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator * (const GeneralCoordinate& lhs, double rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first * rhs;
	ret.body.second = lhs.body.second * rhs;
	ret.leg[0] = lhs.leg[0] * rhs;
	ret.leg[1] = lhs.leg[1] * rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator * (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first * rhs.body.first;
	ret.body.second = lhs.body.second * rhs.body.second;
	ret.leg[0] = lhs.leg[0] * rhs.leg[0];
	ret.leg[1] = lhs.leg[1] * rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator * (double lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs * rhs.body.first;
	ret.body.second = lhs * rhs.body.second;
	ret.leg[0] = lhs * rhs.leg[0];
	ret.leg[1] = lhs * rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
bool operator == (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	return 
		lhs.body.first == rhs.body.first && 
		lhs.body.second == rhs.body.second &&
		lhs.leg[0] == rhs.leg[0] &&
		lhs.leg[1] == rhs.leg[1];
}

//------------------------------------------------------------------------------
void GeneralCoordinate::MakeNear(const GeneralCoordinate& pivot)
{
	ExponentialMap::MakeNearRotation(pivot.body.second, body.second);

	for (int i = 0; i < COUNT_OF(leg); ++i)
	{
		ExponentialMap::MakeNearRotation(pivot.leg[i].rot1, leg[i].rot1);
		ExponentialMap::MakeNearRotation(pivot.leg[i].rot2, leg[i].rot2);
		ExponentialMap::MakeNearRotation(pivot.leg[i].footRot, leg[i].footRot);
	}
}

//--------------------------------------------------------------------------
void GeneralCoordinate::Clear()
{
	body.first.FillZero();
	body.second.FillZero();

	leg[0].FillZero();
	leg[1].FillZero();
}

//--------------------------------------------------------------------------
void GeneralCoordinate::Dump() const
{
#if 0
	WindowsUtility::Debug(
		L"B(%+.3f,%+.3f,%+.3f) (%+.3f,%+.3f,%+.3f) "
		"LR1(%+.3f,%+.3f,%+.3f) LLen1(%+.3f), "
		"LR2(%+.3f, %+.3f,%+.3f) LLen2(%+.3f), "
		"LFR(%+.3f, %+.3f,%+.3f) "
		"RR1(%+.3f,%+.3f,%+.3f) RLen1(%+.3f) "
		"RR2(%+.3f,%+.3f,%+.3f) RLen2(%+.3f) "
		"RFR(%+.3f,%+.3f,%+.3f)\n",
		body.first.x, body.first.y, body.first.z,
		body.second.x, body.second.y, body.second.z,
		leg[0].rot1.x, leg[0].rot1.y, leg[0].rot1.z, leg[0].len1,
		leg[0].rot2.x, leg[0].rot2.y, leg[0].rot2.z, leg[0].len2,
		leg[0].footRot.x, leg[0].footRot.y, leg[0].footRot.z,
		leg[1].rot1.x, leg[1].rot1.y, leg[1].rot1.z, leg[1].len1,
		leg[1].rot2.x, leg[1].rot2.y, leg[1].rot2.z, leg[1].len2,
		leg[1].footRot.x, leg[1].footRot.y, leg[1].footRot.z);
#else
	{
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t"
			"%e\t%e\t%e\t",
			body.first.x, body.first.y, body.first.y,
			body.second.x, body.second.y, body.second.y);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[0].rot1.x, leg[0].rot1.y, leg[0].rot1.z, leg[0].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[0].rot2.x, leg[0].rot2.y, leg[0].rot2.z, leg[0].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t",
			leg[0].footRot.x, leg[0].footRot.y, leg[0].footRot.z);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[1].rot1.x, leg[1].rot1.y, leg[1].rot1.z, leg[1].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[1].rot2.x, leg[1].rot2.y, leg[1].rot2.z, leg[1].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t",
			leg[1].footRot.x, leg[1].footRot.y, leg[1].footRot.z);

		WindowsUtility::Debug(L"\n");
	}
#endif
}

//--------------------------------------------------------------------------
void GeneralCoordinate::Dump_() const
{
	{
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[1].rot2.x, leg[1].rot2.y, leg[1].rot2.z, leg[1].len2);
		WindowsUtility::Debug(L"\n");
	}
}

//--------------------------------------------------------------------------
double GeneralCoordinate::SquaredLength() const
{
	return
		Core::SquaredLength(body.first) +
		Core::SquaredLength(body.second)
		+
		leg[0].SquaredLength() + 
		leg[1].SquaredLength();
}

//--------------------------------------------------------------------------
// 일단 몸, 다리, 발만 계산한다
GeneralCoordinate 
	RobotCoordinate::ToGeneralCoordinate(Robot* robot) const
{
	GeneralCoordinate coord;

	coord.body.first = robot->GetWorldPosition("Body");
	coord.body.second = robot->GetLocalRotation("Body");

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
	const GeneralCoordinate& coord, 
	bool validate) const
{
	bool result = true;

	{
		auto bodyM = ExponentialMap::ToMatrix(coord.body.second);
		FrameHelper::SetTranslation(bodyM, coord.body.first);
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

	coord.body.first = robot->GetWorldPosition("Body");
	coord.body.second = robot->GetLocalRotation("Body");

	coord.foot[0].first = robot->GetWorldPosition("LFoot");
	coord.foot[0].second = ExponentialMap::FromMatrix(robot->GetWorldTransform("LFoot"));

	coord.foot[1].first = robot->GetWorldPosition("RFoot");
	coord.foot[1].second = ExponentialMap::FromMatrix(robot->GetWorldTransform("RFoot"));

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
			auto bodyM = ExponentialMap::ToMatrix(coord.body.second);
			FrameHelper::SetTranslation(bodyM, coord.body.first);
			SetWorld(robot, "Body", bodyM, false);
		}

		robot->SetFootTransform(true, coord.foot[0].first, coord.foot[0].second);
		robot->SetFootTransform(false, coord.foot[1].first, coord.foot[1].second);

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
		body.first.x, body.first.y, body.first.z,
		body.second.x, body.second.y, body.second.z,
		foot[0].first.x, foot[0].first.y, foot[0].first.z,
		foot[0].second.x, foot[0].second.y, foot[0].second.z,
		foot[1].first.x, foot[1].first.y, foot[1].first.z,
		foot[1].second.x, foot[1].second.y, foot[1].second.z);
}