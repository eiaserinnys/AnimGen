#pragma once

#include "Vector.h"
#include "Matrix.h"

#include "Mesh.h"

struct RobotBody
{
	std::string name;

	RobotBody* parent = nullptr;
	int parentIndex;

	std::list<RobotBody*> children;

	Core::Matrix4D linkTx = Core::Matrix4D::Identity();

private:
	Core::Matrix4D localTx = Core::Matrix4D::Identity();
	bool localTxValid = true;

public:
	inline const Core::Matrix4D& LocalTx()
	{ 
		if (!localTxValid)
		{
			CalculateLocalTransform();
			localTxValid = true;
		}
		return localTx; 
	}
	void SetLocalTx(const Core::Matrix4D& tx)
	{
		localTxValid = true;
		localTx = tx;

		worldTxValid = false;

		CalculateLocalRotation();

		InvalidateChildren();
	}

	Core::Vector4D quat = Core::Vector4D(0, 0, 0, 0);
	Core::Vector3D expMap = Core::Vector3D(0, 0, 0);
	Core::Vector4D quatVerify = Core::Vector4D(0, 0, 0, 0);

private:
	Core::Matrix4D worldTx = Core::Matrix4D::Identity();
	bool worldTxValid = true;

public:
	inline const Core::Matrix4D& WorldTx()
	{ 
		if (!worldTxValid)
		{
			CalculateWorldTransform();
			worldTxValid = true;
		}
		return worldTx; 
	}
	void SetWorldTx(const Core::Matrix4D& tx)
	{ 
		worldTxValid = true;
		worldTx = tx;

		localTxValid = false;

		InvalidateChildren();
	}

	std::unique_ptr<IMesh> mesh;

	void CalculateLinkTransform();
	void CalculateWorldTransform();
	void CalculateLocalTransform();
	void CalculateLocalRotation();

	void Update()
	{
		LocalTx();
		WorldTx();
	}

	void InvalidateChildren()
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			(*it)->worldTxValid = false;
		}
	}
};
