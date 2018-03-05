#include "pch.h"
#include "RobotBody.h"

#include <WindowsUtility.h>

#include "FrameHelper.h"
#include "DXMathTransform.h"
#include "ExponentialMap.h"

using namespace std;
using namespace DirectX;
using namespace Core;

//------------------------------------------------------------------------------
void RobotBody::CalculateLocalTransform()
{
	localTx = parent != nullptr ?
		worldTx * parent->InvWorldTx() * invLinkTx :
		worldTx * invLinkTx;

	CalculateLocalRotation();
}

//------------------------------------------------------------------------------
void RobotBody::CalculateWorldTransform()
{
	worldTx = parent != nullptr ?
		localTx * linkTx * parent->WorldTx() :
		localTx * linkTx;

	invWorldTx = DXMathTransform<double>::MatrixInverseHomogeneousTransform(worldTx);
}

//------------------------------------------------------------------------------
void RobotBody::CalculateLocalRotation()
{
	quat = Normalize(DXMathTransform<double>::QuaternionRotationMatrix(localTx));

	auto revQuat = DXMathTransform<double>::MatrixRotationQuaternion(quat);
	FrameHelper::SetTranslation(revQuat, FrameHelper::GetTranslation(localTx));
	if (!revQuat.AssertEqual(localTx))
	{
		// 다시 한 번 더 간다
		revQuat = DXMathTransform<double>::MatrixRotationQuaternion(quat);
	}

	expMap = ExponentialMap::FromQuaternion(quat);

	quatVerify = ExponentialMap::ToQuaternion(expMap);
}

//------------------------------------------------------------------------------
void RobotBody::CalculateLinkTransform()
{
	linkTx = parent != nullptr ? 
		worldTx * parent->InvWorldTx() :
		worldTx;

	invLinkTx = DXMathTransform<double>::MatrixInverseHomogeneousTransform(linkTx);
}
