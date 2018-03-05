#include "pch.h"
#include "Robot.h"

#include <WindowsUtility.h>

#include "DXMathTransform.h"
#include "FrameHelper.h"
#include "AngleHelper.h"

#include "Vector.h"
#include "VectorDXMathAdaptor.h"
#include "Matrix.h"
#include "ExponentialMap.h"

#include "RobotImplementation.h"
#include "RobotBuilder.h"

#define VALIDATE 0

using namespace std;
using namespace Core;
using namespace DirectX;

//--------------------------------------------------------------------------
Robot::Robot()
	: ik(this)
{
	RobotBuilder build(this);
}

//--------------------------------------------------------------------------
void Robot::Update()
{
	UpdateWorldTransform();

	gc = coord.ToGeneralCoordinate(this);
	coord.SetTransform(this, gc, true);

	auto sc = coord.ToSolutionCoordinate(this);
	coord.SetTransform(this, sc, true);

	TransformMesh();
}

//--------------------------------------------------------------------------
void Robot::ResetTransform()
{
	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		auto body = *it;
		body->SetLocalTx(Matrix4D::Identity());
	}
}

//--------------------------------------------------------------------------
void Robot::UpdateWorldTransform()
{
	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		auto body = *it;
		body->Update();
	}
}

//--------------------------------------------------------------------------
void Robot::TransformMesh()
{
	int offset = 0;
	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		auto body = *it;

		auto transform = [&](IMesh* mesh)
		{
			for (size_t i = 0; i < mesh->Vertices().second; ++i)
			{
				pos[offset] =
					ToXMFLOAT3(DXMathTransform<double>::Transform(
						FromXMFLOAT3(mesh->Vertices().first[i]),
						body->WorldTx()));
				nor[offset] = 
					ToXMFLOAT3(DXMathTransform<double>::TransformNormal(
						FromXMFLOAT3(mesh->Normals().first[i]),
						body->WorldTx()));

				offset++;
			}
		};

		transform(body->mesh.get());
		transform(frame.get());
	}
}

//--------------------------------------------------------------------------
int Robot::GetBoneIndex(const string& name) const
{
	auto it = nameToIndex.find(name);
	return it != nameToIndex.end() ? it->second : -1;
}

//--------------------------------------------------------------------------
void Robot::Animate_Test(DWORD elapsed)
{
	total += elapsed;

	double angle = total / 5000.0f * 2 * M_PI;

	double headSwing = sin(angle) * M_PI * 0.3;
	double leg1Swing = sin(angle) * M_PI * 0.3;
	double leg2Swing = (1 - cos(angle)) / 2 * M_PI / 2;
	double armSwing = (1 - cos(angle)) / 2 * M_PI;
	double armSwing2 = (1 - cos(angle)) / 2 * M_PI / 2;

	bodies[GetBoneIndex("Body")]->SetLocalTx(
		DXMathTransform<double>::RotationY(angle / 2) * 
		DXMathTransform<double>::RotationZ(angle));
	bodies[GetBoneIndex("Head")]->SetLocalTx(DXMathTransform<double>::RotationY(headSwing));
	bodies[GetBoneIndex("RArm1")]->SetLocalTx(DXMathTransform<double>::RotationY(-armSwing2) * DXMathTransform<double>::RotationZ(armSwing));
	bodies[GetBoneIndex("RArm2")]->SetLocalTx(DXMathTransform<double>::RotationZ(leg2Swing));
	bodies[GetBoneIndex("LArm1")]->SetLocalTx(DXMathTransform<double>::RotationY(armSwing2) * DXMathTransform<double>::RotationZ(armSwing));
	bodies[GetBoneIndex("LArm2")]->SetLocalTx(DXMathTransform<double>::RotationZ(leg2Swing));

	bodies[GetBoneIndex("RLeg1")]->SetLocalTx(DXMathTransform<double>::RotationZ(leg1Swing));
	bodies[GetBoneIndex("RLeg2")]->SetLocalTx(DXMathTransform<double>::RotationZ(-leg2Swing));
	bodies[GetBoneIndex("LLeg1")]->SetLocalTx(DXMathTransform<double>::RotationZ(-leg1Swing));
	bodies[GetBoneIndex("LLeg2")]->SetLocalTx(DXMathTransform<double>::RotationZ(-leg2Swing));

	// 발 위치를 다시 IK한다 (IK 로직과 다른 좌표로 가기 때문에 검증이 안 됨)
	SetFootPosition(true, FrameHelper::GetTranslation(Find("LFoot")->WorldTx()));
	SetFootPosition(false, FrameHelper::GetTranslation(Find("RFoot")->WorldTx()));
}

//--------------------------------------------------------------------------
RobotBody* Robot::Find(const string& name)
{
	auto index = GetBoneIndex(name);
	if (index >= 0) { return bodies[index]; }
	return nullptr;
}

//--------------------------------------------------------------------------
const RobotBody* Robot::Find(const string& name) const
{
	return const_cast<Robot*>(this)->Find(name);
}

//--------------------------------------------------------------------------
Vector3D Robot::GetWorldPosition(const string& name)
{
	auto index = GetBoneIndex(name);
	if (index >= 0) { return FrameHelper::GetTranslation(bodies[index]->WorldTx()); }
	return Vector3D(0, 0, 0);
}

//--------------------------------------------------------------------------
Matrix4D Robot::GetLinkTransform(const string& name)
{
	auto index = GetBoneIndex(name);
	if (index >= 0) { return bodies[index]->LinkTx(); }
	return Matrix4D::Identity();
}

//--------------------------------------------------------------------------
Matrix4D Robot::GetWorldTransform(const string& name)
{
	auto index = GetBoneIndex(name);
	if (index >= 0) { return bodies[index]->WorldTx(); }
	return Matrix4D::Identity();
}

//--------------------------------------------------------------------------
void Robot::SetFootPosition(bool left, const Vector3D& pos_)
{
	ik.SetFootPosition(left, pos_);
}

//--------------------------------------------------------------------------
void Robot::SetFootTransform(bool left, const Vector3D& pos_, const Vector3D& rot_)
{
	ik.SetFootTransform(left, pos_, rot_);
}

//--------------------------------------------------------------------------
Robot::~Robot()
{
	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		delete *it;
	}
	bodies.clear();
}

//--------------------------------------------------------------------------
const Vector3D Robot::GetLocalRotation(const string& name)
{
	// 먼저 쿼터니언으로 바꾼 뒤
	auto found = Find(name);
	if (found != nullptr) { return found->expMap; }
	return Vector3D(0, 0, 0);
}

//--------------------------------------------------------------------------
const Vector4D Robot::GetLocalQuaternion(const string& name)
{
	// 먼저 쿼터니언으로 바꾼 뒤
	auto found = Find(name);
	if (found != nullptr) { return found->quat; }
	return Vector4D(0, 0, 0, 1);
}

//--------------------------------------------------------------------------
const Vector4D Robot::GetLocalQuaternionVerify(const string& name)
{
	// 먼저 쿼터니언으로 바꾼 뒤
	auto found = Find(name);
	if (found != nullptr) { return found->quatVerify; }
	return Vector4D(0, 0, 0, 1);
}

//------------------------------------------------------------------------------
const SolutionCoordinate Robot::CurrentSC() const
{
	return coord.ToSolutionCoordinate((Robot*)this);
}

//------------------------------------------------------------------------------
void Robot::Apply(const SolutionCoordinate& sc, bool dump)
{
	ResetTransform();

	coord.SetTransform(this, sc, false);

	UpdateWorldTransform();

#	if VALIDATE
	{
		// 검증
		auto sc_ = coord.ToSolutionCoordinate(this);
		coord.SetTransform(this, sc_, true);

		gc = coord.ToGeneralCoordinate(this);
		if (!coord.SetTransform(this, gc, true))
		{
			if (IsDebuggerPresent())
			{
				DebugBreak();
			}
			coord.SetTransform(this, sc, false);
		}
	}
#	else
	{
		gc = coord.ToGeneralCoordinate(this);

		// 에러가 누적되지 않도록 한다
		gc.body = sc.body;
	}
#	endif

	if (dump)
	{
		Dump();
	}
}

//------------------------------------------------------------------------------
void Robot::Dump()
{
	for (int i = 0; i < bodies.size(); ++i)
	{
		auto body = bodies[i];

		auto& k = body->LinkTx();
		auto& l = body->LocalTx();
		auto& w = body->WorldTx();

		WindowsUtility::Debug(L"[%S]\n", body->name.c_str());

		WindowsUtility::Debug(
			L"K(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f)\n",
			k.e[0 * 4 + 0], k.e[0 * 4 + 1], k.e[0 * 4 + 2], k.e[0 * 4 + 3],
			k.e[1 * 4 + 0], k.e[1 * 4 + 1], k.e[1 * 4 + 2], k.e[1 * 4 + 3],
			k.e[2 * 4 + 0], k.e[2 * 4 + 1], k.e[2 * 4 + 2], k.e[2 * 4 + 3],
			k.e[3 * 4 + 0], k.e[3 * 4 + 1], k.e[3 * 4 + 2], k.e[3 * 4 + 3]);

		WindowsUtility::Debug(
			L"L(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f)\n",
			l.e[0 * 4 + 0], l.e[0 * 4 + 1], l.e[0 * 4 + 2], l.e[0 * 4 + 3],
			l.e[1 * 4 + 0], l.e[1 * 4 + 1], l.e[1 * 4 + 2], l.e[1 * 4 + 3],
			l.e[2 * 4 + 0], l.e[2 * 4 + 1], l.e[2 * 4 + 2], l.e[2 * 4 + 3],
			l.e[3 * 4 + 0], l.e[3 * 4 + 1], l.e[3 * 4 + 2], l.e[3 * 4 + 3]);

		WindowsUtility::Debug(
			L"W(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f) "
			"(%.8f, %.8f, %.8f, %.8f)\n",
			w.e[0 * 4 + 0], w.e[0 * 4 + 0], w.e[0 * 4 + 0], w.e[0 * 4 + 0],
			w.e[1 * 4 + 0], w.e[1 * 4 + 0], w.e[1 * 4 + 0], w.e[1 * 4 + 0],
			w.e[2 * 4 + 0], w.e[2 * 4 + 0], w.e[2 * 4 + 0], w.e[2 * 4 + 0],
			w.e[3 * 4 + 0], w.e[3 * 4 + 0], w.e[3 * 4 + 0], w.e[3 * 4 + 0]);
	}
}

//------------------------------------------------------------------------------
IRobot* IRobot::Create()
{ return new Robot; }

//------------------------------------------------------------------------------
Vector3D IRobot::GetFootDirection(const Vector3D& legDir)
{
	return RobotIK::GetFootDirection(legDir, false);
}

