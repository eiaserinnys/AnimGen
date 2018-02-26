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
#include "RobotCoordinate.h"

class Robot : public MeshT<IRobot> {
public:
	Robot();

	~Robot();

	void UpdateWorldTransform();

	void TransformMesh();

	int GetBoneIndex(const std::string& name) const;

	void Animate_Test(DWORD elapsed);

	void Update();

	void ResetTransform();

	RobotBody* Find(const std::string& name);

	const RobotBody* Find(const std::string& name) const;

	Core::Matrix4D GetLinkTransform(const std::string& name);

	Core::Vector3D GetWorldPosition(const std::string& name);
	Core::Matrix4D GetWorldTransform(const std::string& name);

	void SetFootPosition(bool left, const Core::Vector3D& pos_);
	void SetFootTransform(bool left, const Core::Vector3D& pos, const Core::Vector3D& rot);

	const Core::Vector3D GetLocalRotation(const std::string& name);

	const Core::Vector4D GetLocalQuaternion(const std::string& name);

	const Core::Vector4D GetLocalQuaternionVerify(const std::string& name);

	virtual const GeneralCoordinate& Current() const { return gc; }
	virtual const SolutionCoordinate CurrentSC() const;

	virtual void Apply(const SolutionCoordinate& coord, bool dump = false);

	void Dump();

public:
	std::unique_ptr<IMesh> frame;
	std::vector<RobotBody*> bodies;
	std::map<std::string, int> nameToIndex;

	RobotCoordinate coord;
	GeneralCoordinate gc;

	RobotIK ik;

	Core::Vector3D ofs1, ofs2;
	Core::Vector2D legLen;

	DWORD total = 0;
};
