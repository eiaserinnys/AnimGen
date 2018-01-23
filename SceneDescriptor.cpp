#include "pch.h"
#include "SceneDescriptor.h"

#include <WindowsUtility.h>

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
void SceneDescriptor::Build(
	HWND hwnd,
	const XMFLOAT3& eye_, 
	const XMMATRIX& view_)
{
	world = XMMatrixIdentity();

	view = XMMatrixTranspose(view_);

	eye = XMFLOAT4(eye_.x, eye_.y, eye_.z, 1.0f);

	// 프로젝션 매트릭스
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		float zMin = 0.1f;
		float zMax = 20.0f;

		proj = XMMatrixTranspose(
			XMMatrixPerspectiveFovRH(
				45.0 / 2 / 180 * XM_PI,
				width / (FLOAT)height,
				zMin,
				zMax));

		worldViewProj = XMMatrixMultiply(XMMatrixMultiply(proj, view), world);

		invWorldViewT = XMMatrixTranspose(
			XMMatrixInverse(
				nullptr, XMMatrixMultiply(view, world)));
	}
}
