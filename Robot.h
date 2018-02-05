#pragma once

#include "Mesh.h"
#include "Vector.h"

class IRobot : public IMesh {
public:
	struct GeneralCoordinate
	{
		Core::Vector3D bodyPos;
		Core::Vector3D bodyRot;

		Core::Vector3D leg1Rot[2];
		double leg1Len[2];

		Core::Vector3D leg2Rot[2];
		double leg2Len[2];

		Core::Vector3D footRot[2];
	};

	virtual void Animate_Test(DWORD elapsed) = 0;
	virtual void Update() = 0;

	virtual const GeneralCoordinate& Current() const = 0;

	virtual Core::Vector3D GetWorldPosition(const std::string& name) = 0;
	
	virtual void SetFootPosition(bool left, const Core::Vector3D& pos) = 0;

	static Core::Vector3D GetFootDirection(const Core::Vector3D& legDir);

	virtual const double* GetLocalRotation(const std::string& name) = 0;
	virtual const Core::Vector4D GetLocalQuaternion(const std::string& name) = 0;
	virtual const Core::Vector4D GetLocalQuaternionVerify(const std::string& name) = 0;

	static IRobot* Create();
};