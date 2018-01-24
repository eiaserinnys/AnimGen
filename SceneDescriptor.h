#pragma once

#include <DirectXMath.h>

struct SceneDescriptor
{
	DirectX::XMFLOAT3 lightDir;

	DirectX::XMFLOAT4 eye, target;
	
	DirectX::XMMATRIX world, view, proj;
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMMATRIX invWorldViewT;
	DirectX::XMFLOAT2 zRange;

	void Build(
		HWND hwnd,
		const DirectX::XMFLOAT3& eye,
		const DirectX::XMFLOAT3& target,
		const DirectX::XMMATRIX& view);

	std::pair<
		DirectX::XMMATRIX,
		DirectX::XMFLOAT4> GetLightTransform() const;
};
