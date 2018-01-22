#include "pch.h"
#include "SceneDescriptor.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
void SceneDescriptor::Build(
	HWND hwnd,
	const XMFLOAT3& eye_,
	const XMFLOAT3& at_,
	const XMMATRIX& rotation)
{
	world = XMMatrixIdentity();

	float atHeight = 0.85f + 0.15f;
	float eyeHeight = 1.2f + 0.15f;

	// 뷰
	float distance = 7.5f; // 4.8f;
	float zMin = 0.1f;
	float zMax = 20.0f;

	XMVECTOR upVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR atVec = XMVectorSet(at_.x, at_.y, at_.z, 0);
	XMVECTOR eyeVec = XMVectorSet(eye_.x, eye_.y, eye_.z, 0);

	XMVECTOR delta = eyeVec - atVec;

	XMMATRIX rotInv = XMMatrixInverse(nullptr, rotation);

	XMVECTOR upVecT = XMVector3TransformCoord(upVec, rotInv);
	XMVECTOR deltaT = XMVector3TransformCoord(delta, rotInv);

	eyeVec = atVec + deltaT;

	view = XMMatrixTranspose(XMMatrixLookAtRH(eyeVec, atVec, upVec));

	eye = XMFLOAT4(eyeVec.m128_f32);

	// 프로젝션 매트릭스
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		proj = XMMatrixTranspose(
			XMMatrixPerspectiveFovRH(
				45.0 / 2 / 180 * XM_PI,
				width / (FLOAT)height,
				zMin,
				zMax));

		worldViewProj = XMMatrixMultiply(XMMatrixMultiply(proj, view), world);
	}
}
