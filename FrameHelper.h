#pragma once

#include <DirectXMath.h>
#include "Vector.h"
#include "Matrix.h"

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

	static Core::Vector3D GetX(const Core::Matrix4D& tx);
	static Core::Vector3D GetY(const Core::Matrix4D& tx);
	static Core::Vector3D GetZ(const Core::Matrix4D& tx);

	static void SetX(Core::Matrix4D& tx, const Core::Vector3D& v);
	static void SetY(Core::Matrix4D& tx, const Core::Vector3D& v);
	static void SetZ(Core::Matrix4D& tx, const Core::Vector3D& v);

	static void Set(
		Core::Matrix4D& tx,
		const Core::Vector3D& x,
		const Core::Vector3D& y,
		const Core::Vector3D& z);

	static void SetTranslation(Core::Matrix4D& tx, const Core::Vector3D& pos);
	static Core::Vector3D GetTranslation(const Core::Matrix4D& tx);
};