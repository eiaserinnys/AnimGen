#pragma once

#include <DirectXMath.h>

struct SceneDescriptor
{
	DirectX::XMFLOAT3 lightDir;

	DirectX::XMFLOAT4 eye, target;
	
	DirectX::XMMATRIX world, view, proj;
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMMATRIX worldViewProjT;
	DirectX::XMMATRIX invWorldViewT;
	DirectX::XMFLOAT2 zRange;

	UINT width, height;

	void Build(
		HWND hwnd,
		const DirectX::XMFLOAT3& eye,
		const DirectX::XMFLOAT3& target,
		const DirectX::XMMATRIX& view);

	std::pair<
		DirectX::XMMATRIX,
		DirectX::XMFLOAT4> GetLightTransform() const;

	DirectX::XMFLOAT3 GetNdcCoordinate(const DirectX::XMFLOAT3& pos) const;
	DirectX::XMFLOAT3 GetScreenCoordinate(const DirectX::XMFLOAT3& pos) const;
};
