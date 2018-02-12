#pragma once

#include "Mesh.h"
#include "Vector.h"
#include "Matrix.h"

#include "RobotCoordinate.h"

class IRobot : public IMesh {
public:
	virtual void Animate_Test(DWORD elapsed) = 0;
	virtual void Update() = 0;

	virtual const GeneralCoordinate& Current() const = 0;
	virtual const SolutionCoordinate CurrentSC() const = 0;

	virtual Core::Matrix4D GetLinkTransform(const std::string& name) = 0;

	virtual Core::Vector3D GetWorldPosition(const std::string& name) = 0;
	virtual Core::Matrix4D GetWorldTransform(const std::string& name) = 0;

	virtual void SetFootPosition(bool left, const Core::Vector3D& pos) = 0;
	virtual void SetFootTransform(bool left, const Core::Vector3D& pos, const Core::Vector3D& rot) = 0;

	static Core::Vector3D GetFootDirection(const Core::Vector3D& legDir);

	virtual const Core::Vector3D GetLocalRotation(const std::string& name) = 0;
	virtual const Core::Vector4D GetLocalQuaternion(const std::string& name) = 0;
	virtual const Core::Vector4D GetLocalQuaternionVerify(const std::string& name) = 0;

	static IRobot* Create();
};