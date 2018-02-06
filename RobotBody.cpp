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
		localTx = worldTx * parent->InvWorldTx() * invLinkTx;
	}
	else
	{
		localTx = worldTx * invLinkTx;
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

	invWorldTx = worldTx.Inverse();
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
		linkTx = worldTx * parent->InvWorldTx();
	}
	else
	{
		linkTx = worldTx;
	}

	invLinkTx = linkTx.Inverse();
}
