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
	string name;
	int parentIndex;
	XMMATRIX localTx = XMMatrixIdentity();
	XMMATRIX linkTx = XMMatrixIdentity();
	XMMATRIX worldTx = XMMatrixIdentity();
	unique_ptr<IMesh> mesh;
};

class Robot : public MeshT<IRobot> {
public:
	Robot()
	{
		// ȭ��ǥ�� �ϳ��� ����
		{
			float len = 0.6f;
			float headLen = 0.2f;
			float r1 = 0.0125f;
			float r2 = 0.025f;
			frame.reset(ICoordinateAxisMesh::Create(
				XMMatrixIdentity(), len, headLen, r1, r2, 8));
		}

		DWORD color = 0x0aa0a0ff;

		XMMATRIX leg = XMMatrixRotationZ(-90.0f / 180 * M_PI);
		float margin = 0.05f;

		struct BodyDesc
		{
			string name;
			string parentName;
			XMFLOAT3 position;
			XMFLOAT3 extent;
			XMMATRIX worldTx;
		};

		BodyDesc desc[] =
		{

			// ����
			{ "Body", string(), XMFLOAT3(0, 0, 0), XMFLOAT3(0.25f, 0.61f, 0.4f), XMMatrixTranslation(0, 1.22f, 0) },

			// �Ӹ�
			{ "Head", "Body", XMFLOAT3(0, 0.125f, 0), XMFLOAT3(0.22f, 0.25f, 0.22f), XMMatrixTranslation(0, 1.54f, 0) },

			// ��������
			{ "RArm1", "Body", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.53f - margin, 0.265f) },

			// ���� �Ʒ���
			{ "RArm2", "RArm1", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.17f - margin, 0.265f) },

			// ������
			{ "LArm1", "Body", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.53f - margin, -0.265f) },

			// �޾Ʒ���
			{ "LArm2", "LArm1", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.17f - margin, -0.265f) },

			// ���� �����
			{ "RLeg1", "Body", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.17f, 0.17f), leg * XMMatrixTranslation(0, 0.91f - margin, 0.115f) },

			// ���� ���Ƹ�
			{ "RLeg2", "RLeg1", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.16f, 0.16f), leg * XMMatrixTranslation(0, 0.5f - margin, 0.1f) },

			// ���� ��
			{ "RFoot", "RLeg2", XMFLOAT3(0.05f, 0, 0), XMFLOAT3(0.25f, 0.09f, 0.16f), XMMatrixTranslation(0.00f, 0.045f, 0.1f) },

			// �� �����
			{ "LLeg1", "Body", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.17f, 0.17f), leg * XMMatrixTranslation(0, 0.91f - margin, -0.115f) },

			// �� ���Ƹ�
			{ "LLeg2", "LLeg1", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.16f, 0.16f), leg * XMMatrixTranslation(0, 0.5f - margin, -0.1f) },

			// �޹�
			{ "LFoot", "LLeg2", XMFLOAT3(0.05f, 0, 0), XMFLOAT3(0.25f, 0.09f, 0.16f), XMMatrixTranslation(0.00f, 0.045f, -0.1f) },
		};

		for (int i = 0; i < COUNT_OF(desc); ++i)
		{
			CreateBody(
				desc[i].name, 
				desc[i].parentName, 
				desc[i].position, 
				desc[i].extent, 
				desc[i].worldTx);
		}

		BuildLinkMatrix();

		UpdateWorldTransform();

		TransformMesh();
	}

	void CreateBody(
		const string& name, 
		const string& parentName,
		const XMFLOAT3& pos, 
		const XMFLOAT3& size, 
		const XMMATRIX& worldTx)
	{
		if (nameToIndex.find(name) != nameToIndex.end())
		{
			throw invalid_argument("���� �̸��� ���� �ֽ��ϴ�.");
		}

		int parIndex = -1;
		if (!parentName.empty())
		{
			auto it = nameToIndex.find(parentName);
			if (it == nameToIndex.end())
			{
				throw invalid_argument("�־��� �θ� �̸��� ���� ã�� �� �����ϴ�.");
			}
			parIndex = it->second;
		}

		DWORD color = 0x0aa0a0ff;

		nameToIndex.insert(make_pair(name, bodies.size()));

		auto body = new RobotBody;
		body->name = name;
		body->parentIndex = parIndex;
		body->mesh.reset(IBoxMesh::Create(pos, size, color));
		body->worldTx = worldTx;

		bodies.push_back(body);
		Append(body->mesh.get());
		Append(frame.get());
	}

	void BuildLinkMatrix()
	{
		// ���� Ʈ�������� ���ٰ� �����ϸ�
		// (�� ��ũ) x (�θ��� ����) = (�� ����)
		// (�� ��ũ) = (�� ����) x (�θ��� ����)^(-1)

		for (size_t i = 0; i < bodies.size(); ++i)
		{
			auto body = bodies[i];
			if (body->parentIndex >= 0)
			{
				body->linkTx =
					body->worldTx *
					XMMatrixInverse(nullptr, bodies[body->parentIndex]->worldTx);
			}
			else
			{
				body->linkTx = body->worldTx;
			}
		}
	}

	void UpdateWorldTransform()
	{
		for (auto it = bodies.begin(); it != bodies.end(); ++it)
		{
			auto body = *it;

			// (�� ����) = (�� ����) x (�� ��ũ) x (�θ��� ����)
			if (body->parentIndex >= 0)
			{
				body->worldTx = body->localTx * body->linkTx * bodies[body->parentIndex]->worldTx;
			}
			else
			{
				body->worldTx = body->localTx * body->linkTx;
			}
		}
	}

	void TransformMesh()
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

	int GetBoneIndex(const string& name) const
	{
		auto it = nameToIndex.find(name);
		return it != nameToIndex.end() ? it->second : -1;
	}

	void Update_Test(DWORD elapsed)
	{
		total += elapsed;

		float angle = total / 5000.0f * 2 * M_PI;

		float headSwing = sin(angle) * M_PI * 0.3f;
		float leg1Swing = sin(angle) * M_PI * 0.3f;
		float leg2Swing = (1 - cos(angle)) / 2 * M_PI / 2;
		float armSwing = (1 - cos(angle)) / 2 * M_PI;
		float armSwing2 = (1 - cos(angle)) / 2 * M_PI / 2;

		//bodies[GetBoneIndex("Body")]->localTx = XMMatrixRotationY(angle);
		bodies[GetBoneIndex("Head")]->localTx = XMMatrixRotationY(headSwing);
		bodies[GetBoneIndex("RArm1")]->localTx = XMMatrixRotationY(-armSwing2) * XMMatrixRotationZ(armSwing);
		bodies[GetBoneIndex("RArm2")]->localTx = XMMatrixRotationZ(leg2Swing);
		bodies[GetBoneIndex("LArm1")]->localTx = XMMatrixRotationY(armSwing2) * XMMatrixRotationZ(armSwing);
		bodies[GetBoneIndex("LArm2")]->localTx = XMMatrixRotationZ(leg2Swing);

		bodies[GetBoneIndex("RLeg1")]->localTx = XMMatrixRotationZ(leg1Swing);
		bodies[GetBoneIndex("RLeg2")]->localTx = XMMatrixRotationZ(-leg2Swing);
		bodies[GetBoneIndex("LLeg1")]->localTx = XMMatrixRotationZ(-leg1Swing);
		bodies[GetBoneIndex("LLeg2")]->localTx = XMMatrixRotationZ(-leg2Swing);

		UpdateWorldTransform();

		TransformMesh();
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
	unique_ptr<IMesh> frame;
	vector<RobotBody*> bodies;
	map<string, int> nameToIndex;

	DWORD total = 0;
};

IRobot* IRobot::Create()
{ return new Robot; }