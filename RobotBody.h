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

private:
	Core::Matrix4D linkTx = Core::Matrix4D::Identity();
	Core::Matrix4D invLinkTx = Core::Matrix4D::Identity();

	Core::Matrix4D localTx = Core::Matrix4D::Identity();
	bool localTxValid = true;

	Core::Matrix4D worldTx = Core::Matrix4D::Identity();
	Core::Matrix4D invWorldTx = Core::Matrix4D::Identity();
	bool worldTxValid = true;

public:
	Core::Vector4D quat = Core::Vector4D(0, 0, 0, 0);
	Core::Vector3D expMap = Core::Vector3D(0, 0, 0);
	Core::Vector4D quatVerify = Core::Vector4D(0, 0, 0, 0);

	std::unique_ptr<IMesh> mesh;

public:
	inline const Core::Matrix4D& LinkTx()
	{
		return linkTx;
	}

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

	void SetWorldTx(const Core::Matrix4D& tx)
	{
		worldTxValid = true;
		worldTx = tx;
		invWorldTx = tx.Inverse();

		localTxValid = false;

		InvalidateChildren();
	}

	inline const Core::Matrix4D& WorldTx()
	{ 
		ValidateWorldTransform();
		return worldTx; 
	}

	inline const Core::Matrix4D& InvWorldTx()
	{
		ValidateWorldTransform();
		return invWorldTx;
	}

	inline void ValidateWorldTransform()
	{
		if (!worldTxValid)
		{
			CalculateWorldTransform();
			worldTxValid = true;
		}
	}

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
			(*it)->InvalidateChildren();
		}
	}
};
