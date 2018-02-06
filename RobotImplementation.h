#pragma once

#include <map>

#include "Vector.h"
#include "Matrix.h"

#include "Mesh.h"
#include "MeshT.h"
#include "ObjectBuffer.h"

#include "Robot.h"
#include "RobotIK.h"
#include "RobotBody.h"

class Robot : public MeshT<IRobot> {
public:
	Robot();

	~Robot();

	void BuildLinkMatrix();

	void UpdateWorldTransform();

	void TransformMesh();

	int GetBoneIndex(const std::string& name) const;

	void Animate_Test(DWORD elapsed);

	void Update();

	RobotBody* Find(const std::string& name);

	const RobotBody* Find(const std::string& name) const;

	Core::Vector3D GetWorldPosition(const std::string& name);

	void SetFootPosition(bool left, const Core::Vector3D& pos_);

	void CalculateGeneralCoordinate();

	const Core::Vector3D GetLocalRotation(const std::string& name);

	const Core::Vector4D GetLocalQuaternion(const std::string& name);

	const Core::Vector4D GetLocalQuaternionVerify(const std::string& name);

	virtual const GeneralCoordinate& Current() const { return coord; }

public:
	std::unique_ptr<IMesh> frame;
	std::vector<RobotBody*> bodies;
	std::map<std::string, int> nameToIndex;

	GeneralCoordinate coord;
	RobotIK ik;

	Core::Vector2D legLen;

	DWORD total = 0;
};
