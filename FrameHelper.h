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
};