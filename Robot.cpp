#include "pch.h"
#include "Robot.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "MeshT.h"
#include "ObjectBuffer.h"

using namespace std;
using namespace DirectX;

struct RobotBody
{
	XMMATRIX localTx = XMMatrixIdentity();
	XMMATRIX worldTx = XMMatrixIdentity();
	unique_ptr<IMesh> mesh;
};

class Robot : public MeshT<IRobot> {
public:
	unique_ptr<IMesh> frame;

	Robot()
	{
		// 화살표는 하나만 쓴다
		{
			float len = 0.6f;
			float headLen = 0.2f;
			float r1 = 0.0125f;
			float r2 = 0.025f;
			frame.reset(ICoordinateAxisMesh::Create(
				XMMatrixIdentity(), len, headLen, r1, r2, 8));
		}

		DWORD color = 0x0aa0a0ff;

		// 몸통
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(XMFLOAT3(0, 0, 0), XMFLOAT3(0.25f, 0.61f, 0.4f), color));
			body->worldTx = XMMatrixTranslation(0, 1.22f, 0);
		
			AppendBody(body);
		}

		// 머리
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(XMFLOAT3(0, 0.125f, 0), XMFLOAT3(0.22f, 0.25f, 0.22f), color));
			body->worldTx = XMMatrixTranslation(0, 1.54f, 0);
			AppendBody(body);
		}

		XMMATRIX leg = XMMatrixRotationZ(-90.0f / 180 * M_PI);
		float margin = 0.05f;

		// 오른 허벅지
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.17f, 0.17f), color));
			body->worldTx = leg * XMMatrixTranslation(0, 0.91f - margin, 0.115f);
			AppendBody(body);
		}

		// 왼 허벅지
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.17f, 0.17f), color));
			body->worldTx = leg * XMMatrixTranslation(0, 0.91f - margin, -0.115f);
			AppendBody(body);
		}

		// 오른 종아리
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.16f, 0.16f), color));
			body->worldTx = leg * XMMatrixTranslation(0, 0.5f - margin, 0.1f);
			AppendBody(body);
		}

		// 왼 종아리
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.16f, 0.16f), color));
			body->worldTx = leg * XMMatrixTranslation(0, 0.5f - margin, - 0.1f);
			AppendBody(body);
		}

		// 오른 발
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(
				XMFLOAT3(0.05f, 0, 0), 
				XMFLOAT3(0.25f, 0.09f, 0.16f), color));
			body->worldTx = XMMatrixTranslation(0.00f, 0.045f, 0.1f);
			AppendBody(body);
		}

		// 왼발
		{
			auto body = new RobotBody;
			body->mesh.reset(IBoxMesh::Create(
				XMFLOAT3(0.05f, 0, 0),
				XMFLOAT3(0.25f, 0.09f, 0.16f), color));
			body->worldTx = XMMatrixTranslation(0.00f, 0.045f, -0.1f);
			AppendBody(body);
		}

		Transform();
	}

	void AppendBody(RobotBody* body)
	{
		bodies.push_back(body);
		Append(body->mesh.get());
		Append(frame.get());
	}

	void Transform()
	{
		int offset = 0;
		for (auto it = bodies.begin(); it != bodies.end(); ++it)
		{
			auto body = *it;

			auto transform = [&](IMesh* mesh)
			{
				for (int i = 0; i < mesh->Vertices().second; ++i)
				{
					XMStoreFloat3(
						&pos[offset],
						XMVector3Transform(
							XMLoadFloat3(&mesh->Vertices().first[i]),
							body->worldTx));
					XMStoreFloat3(
						&nor[offset],
						XMVector3TransformNormal(
							XMLoadFloat3(&mesh->Normals().first[i]),
							body->worldTx));

					offset++;
				}
			};

			transform(body->mesh.get());
			transform(frame.get());
		}
	}

	~Robot()
	{
		for (auto it = bodies.begin(); it != bodies.end(); ++it)
		{
			delete *it;
		}
		bodies.clear();
	}

protected:
	vector<RobotBody*> bodies;
};

IRobot* IRobot::Create()
{ return new Robot; }