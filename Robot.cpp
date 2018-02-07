#include "pch.h"
#include "Robot.h"

#include "DXMathTransform.h"
#include "FrameHelper.h"
#include "AngleHelper.h"

#include "Vector.h"
#include "VectorDXMathAdaptor.h"
#include "Matrix.h"
#include "ExponentialMap.h"

#include "RobotImplementation.h"
#include "RobotBuilder.h"

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
	// 즉시 검증한다
	coord.FromGeneralCoordinate(this, gc);
	TransformMesh();
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
IRobot* IRobot::Create()
{ return new Robot; }

//------------------------------------------------------------------------------
Vector3D IRobot::GetFootDirection(const Vector3D& legDir)
{
	return RobotIK::GetFootDirection(legDir);
}
