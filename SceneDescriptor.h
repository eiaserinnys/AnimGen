#pragma once

#include <DirectXMath.h>

struct SceneDescriptor
{
	DirectX::XMFLOAT4 eye;
	DirectX::XMMATRIX world, view, proj;
	DirectX::XMMATRIX worldViewProj;

	void Build(
		HWND hwnd,
		const DirectX::XMFLOAT3& eye,
		const DirectX::XMFLOAT3& at,
		const DirectX::XMMATRIX& rotation);
};
