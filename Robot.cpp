#include "pch.h"
#include "Robot.h"

#include "FrameHelper.h"
#include "AngleHelper.h"

#include "Vector.h"
#include "VectorDXMathAdaptor.h"
#include "Matrix.h"
#include "exp-map.h"

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
void Robot::BuildLinkMatrix()
{
	// 로컬 트랜스폼이 없다고 가정하면
	// (내 링크) x (부모의 월드) = (내 월드)
	// (내 링크) = (내 월드) x (부모의 월드)^(-1)

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		auto body = bodies[i];
		if (body->parentIndex >= 0)
		{
			body->linkTx =
				body->worldTx *
				bodies[body->parentIndex]->worldTx.Inverse();
		}
		else
		{
			body->linkTx = body->worldTx;
		}
	}
}

//--------------------------------------------------------------------------
void Robot::CalculateLocalRotation()
{
	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		auto body = *it;

		CalculateLocalRotation(body);
	}
}

//--------------------------------------------------------------------------
void Robot::UpdateWorldTransform()
{
	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		auto body = *it;

		// (내 월드) = (내 로컬) x (내 링크) x (부모의 월드)
		if (body->parentIndex >= 0)
		{
			body->worldTx = body->localTx * body->linkTx * bodies[body->parentIndex]->worldTx;
		}
		else
		{
			body->worldTx = body->localTx * body->linkTx;
		}
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
						body->worldTx));
				nor[offset] = 
					ToXMFLOAT3(DXMathTransform<double>::TransformNormal(
						FromXMFLOAT3(mesh->Normals().first[i]),
						body->worldTx));

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

	bodies[GetBoneIndex("Body")]->localTx = DXMathTransform<double>::RotationZ(angle);
	bodies[GetBoneIndex("Head")]->localTx = DXMathTransform<double>::RotationY(headSwing);
	bodies[GetBoneIndex("RArm1")]->localTx = DXMathTransform<double>::RotationY(-armSwing2) * DXMathTransform<double>::RotationZ(armSwing);
	bodies[GetBoneIndex("RArm2")]->localTx = DXMathTransform<double>::RotationZ(leg2Swing);
	bodies[GetBoneIndex("LArm1")]->localTx = DXMathTransform<double>::RotationY(armSwing2) * DXMathTransform<double>::RotationZ(armSwing);
	bodies[GetBoneIndex("LArm2")]->localTx = DXMathTransform<double>::RotationZ(leg2Swing);

	bodies[GetBoneIndex("RLeg1")]->localTx = DXMathTransform<double>::RotationZ(leg1Swing);
	bodies[GetBoneIndex("RLeg2")]->localTx = DXMathTransform<double>::RotationZ(-leg2Swing);
	bodies[GetBoneIndex("LLeg1")]->localTx = DXMathTransform<double>::RotationZ(-leg1Swing);
	bodies[GetBoneIndex("LLeg2")]->localTx = DXMathTransform<double>::RotationZ(-leg2Swing);
}

//--------------------------------------------------------------------------
void Robot::Update()
{
	CalculateLocalRotation();
	UpdateWorldTransform();
	TransformMesh();
}

//--------------------------------------------------------------------------
RobotBody* Robot::Find(const string& name)
{
	auto index = GetBoneIndex(name);
	if (index > 0) { return bodies[index]; }
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
	if (index > 0)
	{
		auto& worldTx = bodies[index]->worldTx;
		return FrameHelper::GetTranslation(worldTx);
	}
	return Vector3D(0, 0, 0);
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
void Robot::CalculateLocalTransform(int index)
{
	auto& localTx = bodies[index]->localTx;

	if (bodies[index]->parentIndex >= 0)
	{
		auto parent = bodies[bodies[index]->parentIndex];
		localTx = bodies[index]->worldTx *
			parent->worldTx.Inverse() *
			bodies[index]->linkTx.Inverse();
	}
	else
	{
		localTx = bodies[index]->worldTx *
			bodies[index]->linkTx.Inverse();
	}
}

//--------------------------------------------------------------------------
// 일단 몸, 다리, 발만 계산한다
void Robot::CalculateGeneralCoordinate()
{
	coord.bodyPos = GetWorldPosition("Body");
	//coord.bodyROt = GetLocalRotation("Body");
}

//--------------------------------------------------------------------------
const double* Robot::GetLocalRotation(const string& name)
{
	// 먼저 쿼터니언으로 바꾼 뒤
	auto found = Find(name);
	if (found != nullptr) { return found->expMap; }

	static double identity[] = { 0, 0, 0 };
	return identity;
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
void Robot::CalculateLocalRotation(RobotBody* found)
{
	static const double epsilon = sqrt(sqrt(0.000001f));

	auto& quat = found->quat;

	quat = Normalize(DXMathTransform<double>::QuaternionRotationMatrix(found->localTx));

	double theta = acos(quat.w) * 2;

	double sinHalfTheta = sin(theta / 2);

	double m = 0;

	if (abs(theta) < epsilon)
	{
		// 테일러 전개에 의해서
		m = 1 / (1.0 / 2 + theta * theta / 48);
	}
	else
	{
		m = theta / sinHalfTheta;
	}

	found->expMap[0] = m * quat.x;
	found->expMap[1] = m * quat.y;
	found->expMap[2] = m * quat.z;

	double quatV[4] = { 0, 0, 0, 0 };

	EM_To_Q(found->expMap, quatV, 0);

	found->quatVerify.x = quatV[0];
	found->quatVerify.y = quatV[1];
	found->quatVerify.z = quatV[2];
	found->quatVerify.w = quatV[3];
}

//------------------------------------------------------------------------------
IRobot* IRobot::Create()
{ return new Robot; }

Vector3D IRobot::GetFootDirection(const Vector3D& legDir)
{
	return RobotIK::GetFootDirection(legDir);
}
