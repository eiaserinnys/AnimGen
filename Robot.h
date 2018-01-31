#pragma once

#include "Mesh.h"

class IRobot : public IMesh {
public:
	struct GeneralCoordinate
	{
		DirectX::XMFLOAT3 bodyPos;
		DirectX::XMFLOAT3 bodyRot;

		DirectX::XMFLOAT3 leg1Rot[2];
		float leg1Len[2];

		DirectX::XMFLOAT3 leg2Rot[2];
		float leg2Len[2];

		DirectX::XMFLOAT3 footRot[2];
	};

	virtual void Animate_Test(DWORD elapsed) = 0;
	virtual void Update() = 0;

	virtual const GeneralCoordinate& Current() const = 0;

	virtual DirectX::XMFLOAT3 GetWorldPosition(const std::string& name) = 0;
	
	virtual void SetFootPosition(bool left, const DirectX::XMFLOAT3& pos) = 0;

	static DirectX::XMFLOAT3 GetFootDirection(const DirectX::XMFLOAT3& legDir);

	virtual const double* GetLocalRotation(const std::string& name) = 0;
	virtual const DirectX::XMFLOAT4 GetLocalQuaternion(const std::string& name) = 0;
	virtual const DirectX::XMFLOAT4 GetLocalQuaternionVerify(const std::string& name) = 0;

	static IRobot* Create();
};