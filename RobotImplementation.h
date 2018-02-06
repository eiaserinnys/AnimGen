#pragma once

#include <map>

#include "Vector.h"
#include "Matrix.h"

#include "Mesh.h"
#include "MeshT.h"
#include "ObjectBuffer.h"

#include "Robot.h"
#include "RobotIK.h"

struct RobotBody
{
	std::string name;
	int parentIndex;
	Core::Matrix4D linkTx = Core::Matrix4D::Identity();

	Core::Matrix4D localTx = Core::Matrix4D::Identity();
	Core::Vector4D quat = Core::Vector4D(0, 0, 0, 0);
	double expMap[3] = { 0, 0, 0 };
	Core::Vector4D quatVerify = Core::Vector4D(0, 0, 0, 0);

	Core::Matrix4D worldTx = Core::Matrix4D::Identity();
	std::unique_ptr<IMesh> mesh;
};

class Robot : public MeshT<IRobot> {
public:
	Robot();

	~Robot();

	void BuildLinkMatrix();

	void CalculateLocalRotation();

	void UpdateWorldTransform();

	void TransformMesh();

	int GetBoneIndex(const std::string& name) const;

	void Animate_Test(DWORD elapsed);

	void Update();

	RobotBody* Find(const std::string& name);

	const RobotBody* Find(const std::string& name) const;

	Core::Vector3D GetWorldPosition(const std::string& name);

	void SetFootPosition(bool left, const Core::Vector3D& pos_);

	void CalculateLocalTransform(int index);

	void CalculateGeneralCoordinate();

	const double* GetLocalRotation(const std::string& name);

	const Core::Vector4D GetLocalQuaternion(const std::string& name);

	const Core::Vector4D GetLocalQuaternionVerify(const std::string& name);

	void CalculateLocalRotation(RobotBody* found);

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
