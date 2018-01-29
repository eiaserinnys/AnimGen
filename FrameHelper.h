#pragma once

#include <DirectXMath.h>

class FrameHelper {
public:
	static DirectX::XMFLOAT3 GetX(const DirectX::XMMATRIX& tx);
	static DirectX::XMFLOAT3 GetY(const DirectX::XMMATRIX& tx);
	static DirectX::XMFLOAT3 GetZ(const DirectX::XMMATRIX& tx);

	static void SetX(DirectX::XMMATRIX& tx, const DirectX::XMFLOAT3& v);
	static void SetY(DirectX::XMMATRIX& tx, const DirectX::XMFLOAT3& v);
	static void SetZ(DirectX::XMMATRIX& tx, const DirectX::XMFLOAT3& v);

	static void Set(
		DirectX::XMMATRIX& tx,
		const DirectX::XMFLOAT3& x,
		const DirectX::XMFLOAT3& y,
		const DirectX::XMFLOAT3& z);

	static void SetTranslation(DirectX::XMMATRIX& tx, const DirectX::XMFLOAT3& pos);
	static DirectX::XMFLOAT3 GetTranslation(const DirectX::XMMATRIX& tx);
};