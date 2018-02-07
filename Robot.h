#pragma once

#include "Mesh.h"
#include "Vector.h"
#include "Matrix.h"

class IRobot : public IMesh {
public:
	struct GeneralCoordinate
	{
		struct Leg
		{
			Core::Vector3D rot1;
			double len1;

			Core::Vector3D rot2;
			double len2;

			Core::Vector3D footRot;
		};

		Core::Vector3D bodyPos;
		Core::Vector3D bodyRot;

		Leg leg[2];
	};

	virtual void Animate_Test(DWORD elapsed) = 0;
	virtual void Update() = 0;

	virtual const GeneralCoordinate& Current() const = 0;

	virtual Core::Matrix4D GetLinkTransform(const std::string& name) = 0;

	virtual Core::Vector3D GetWorldPosition(const std::string& name) = 0;
	virtual Core::Matrix4D GetWorldTransform(const std::string& name) = 0;

	virtual void SetFootPosition(bool left, const Core::Vector3D& pos) = 0;

	static Core::Vector3D GetFootDirection(const Core::Vector3D& legDir);

	virtual const Core::Vector3D GetLocalRotation(const std::string& name) = 0;
	virtual const Core::Vector4D GetLocalQuaternion(const std::string& name) = 0;
	virtual const Core::Vector4D GetLocalQuaternionVerify(const std::string& name) = 0;

	static IRobot* Create();
};