#include "pch.h"
#include "SceneDescriptor.h"

#include <WindowsUtility.h>
#include <Utility.h>

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
void SceneDescriptor::Build(
	HWND hwnd,
	const XMFLOAT3& eye_, 
	const XMFLOAT3& target_,
	const XMMATRIX& view_)
{
	zRange = XMFLOAT2(0.5f, 30.0f);

	lightDir = Normalize(XMFLOAT3(-2, -2, -1));

	world = XMMatrixIdentity();

	view = XMMatrixTranspose(view_);

	eye = XMFLOAT4(eye_.x, eye_.y, eye_.z, 1.0f);
	target = XMFLOAT4(target_.x, target_.y, target_.z, 1.0f);

	// 프로젝션 매트릭스
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		proj = XMMatrixTranspose(
			XMMatrixPerspectiveFovRH(
				45.0 / 2 / 180 * XM_PI,
				width / (FLOAT)height,
				zRange.x,
				zRange.y));

		worldViewProj = XMMatrixMultiply(XMMatrixMultiply(proj, view), world);

		invWorldViewT = XMMatrixTranspose(
			XMMatrixInverse(
				nullptr, XMMatrixMultiply(view, world)));

		worldViewProjT = XMMatrixTranspose(worldViewProj);

		invWorldViewProjT = XMMatrixInverse(nullptr, worldViewProjT);
	}
}

//------------------------------------------------------------------------------
pair<XMMATRIX, XMFLOAT4> 
	SceneDescriptor::GetLightTransform() const
{
	XMMATRIX proj = XMMatrixTranspose(
		XMMatrixOrthographicRH(
			10.0f,
			10.0f,
			1.0f, 30.0f));

	float lightDist = 10.0f;

	XMFLOAT3 lightEye = 
		XMFLOAT3(target.x, target.y, target.z) - 
		lightDist * lightDir;

	XMVECTOR lightEyeV = XMLoadFloat3(&lightEye);
	XMVECTOR lightTargetV = XMLoadFloat4(&target);
	XMMATRIX view = XMMatrixTranspose(
		XMMatrixLookAtRH(
			lightEyeV,
			lightTargetV,
			XMLoadFloat3(&XMFLOAT3(0, 1, 0))));

	return make_pair(
		XMMatrixMultiply(proj, view),
		XMFLOAT4(lightEye.x, lightEye.y, lightEye.z, 1));
}

//------------------------------------------------------------------------------
XMFLOAT4 SceneDescriptor::GetNdc(const XMFLOAT3& pos) const
{
	XMFLOAT4 posProj;

	XMStoreFloat4(
		&posProj,
		XMVector3Transform(XMLoadFloat3(&pos), worldViewProjT));

	return posProj;
}

//------------------------------------------------------------------------------
XMFLOAT3 SceneDescriptor::GetWorldPositionByNdc(const XMFLOAT4& ndc) const
{
	XMFLOAT3 posProj;

	XMStoreFloat3(
		&posProj,
		XMVector4Transform(XMLoadFloat4(&ndc), invWorldViewProjT));

	return posProj;
}

//------------------------------------------------------------------------------
XMFLOAT3 SceneDescriptor::GetScreenCoordinate(const XMFLOAT3& pos) const
{
	auto posProj = GetNdc(pos);

	return XMFLOAT3(
		(1 + posProj.x / posProj.w) / 2 * width,
		(1 - posProj.y / posProj.w) / 2 * height,
		pos.z);
}

//------------------------------------------------------------------------------
XMFLOAT4 SceneDescriptor::GetNdcByScreenCoordinate(
	const XMFLOAT2& pos,
	float w) const
{
	float z = -w;

	XMFLOAT4 ndc(
		((pos.x / width) * 2 - 1) * w,
		(1 - (pos.y / height) * 2) * w,
		(proj.r[2].m128_f32[2] * z + proj.r[2].m128_f32[3]),
		w);

	return ndc;
}

//------------------------------------------------------------------------------
XMFLOAT3 SceneDescriptor::GetWorldPositionByScreenCoordinate(
	const XMFLOAT2& pos,
	float w) const
{
	auto ndc = GetNdcByScreenCoordinate(pos, w);
	return GetWorldPositionByNdc(ndc);
}
