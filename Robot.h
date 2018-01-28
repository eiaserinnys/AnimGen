#pragma once

#include "Mesh.h"

class IRobot : public IMesh {
public:
	virtual void Animate_Test(DWORD elapsed) = 0;
	virtual void Update() = 0;

	virtual DirectX::XMFLOAT3 GetWorldPosition(const std::string& name) = 0;
	
	virtual void SetFootPosition(bool left, const DirectX::XMFLOAT3& pos) = 0;

	static IRobot* Create();
};