#include "pch.h"
#include "RobotBody.h"

#include "ExponentialMap.h"

using namespace std;
using namespace DirectX;
using namespace Core;

void RobotBody::CalculateLocalTransform()
{
	if (parent != nullptr)
	{
		Matrix4D parInv = parent->WorldTx().Inverse();
		Matrix4D linkInv = linkTx.Inverse();
		localTx = worldTx * parInv * linkInv;
	}
	else
	{
		localTx = worldTx * linkTx.Inverse();
	}

	CalculateLocalRotation();
}

void RobotBody::CalculateWorldTransform()
{
	if (parent != nullptr)
	{
		worldTx = localTx * linkTx * parent->WorldTx();
	}
	else
	{
		worldTx = localTx * linkTx;
	}
}

void RobotBody::CalculateLocalRotation()
{
	quat = Normalize(DXMathTransform<double>::QuaternionRotationMatrix(localTx));

	expMap = ExponentialMap::FromQuaternion(quat);

	quatVerify = ExponentialMap::ToQuaternion(expMap);
}

void RobotBody::CalculateLinkTransform()
{
	if (parent != nullptr)
	{
		linkTx = worldTx * parent->worldTx.Inverse();
	}
	else
	{
		linkTx = worldTx;
	}
}