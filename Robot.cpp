#include "pch.h"
#include "Robot.h"

#include "FrameHelper.h"
#include "AngleHelper.h"

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
		// 화살표는 하나만 쓴다
		{
			float len = 0.3f;
			float headLen = 0.1f;
			float r1 = 0.0125f / sqrtf(2);
			float r2 = 0.025f / sqrtf(2);
			frame.reset(ICoordinateAxisMesh::Create(
				XMMatrixIdentity(), len, headLen, r1, r2, 8));
		}

		DWORD color = 0x0aa0a0ff;

		XMMATRIX leg = XMMatrixRotationZ(-90.0f / 180 * (float)M_PI);
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

			// 몸통
			{ "Body", string(), XMFLOAT3(0, 0, 0), XMFLOAT3(0.25f, 0.61f, 0.4f), XMMatrixTranslation(0, 1.22f, 0) },

			// 머리
			{ "Head", "Body", XMFLOAT3(0, 0.125f, 0), XMFLOAT3(0.22f, 0.25f, 0.22f), XMMatrixTranslation(0, 1.54f, 0) },

			// 오른윗팔
			{ "RArm1", "Body", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.53f - margin, 0.265f) },

			// 오른 아랫팔
			{ "RArm2", "RArm1", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.17f - margin, 0.265f) },

			// 왼윗팔
			{ "LArm1", "Body", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.53f - margin, -0.265f) },

			// 왼아랫팔
			{ "LArm2", "LArm1", XMFLOAT3(0.175f - margin, 0, 0), XMFLOAT3(0.35f, 0.11f, 0.11f), leg * XMMatrixTranslation(0, 1.17f - margin, -0.265f) },

			// 오른 허벅지
			{ "RLeg1", "Body", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.17f, 0.17f), leg * XMMatrixTranslation(0, 0.91f - margin, 0.115f) },

			// 오른 종아리
			{ "RLeg2", "RLeg1", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.16f, 0.16f), leg * XMMatrixTranslation(0, 0.5f - margin, 0.1f) },

			// 오른 발
			{ "RFoot", "RLeg2", XMFLOAT3(0.05f, 0, 0), XMFLOAT3(0.25f, 0.09f, 0.16f), XMMatrixTranslation(0.00f, 0.045f, 0.1f) },

			// 왼 허벅지
			{ "LLeg1", "Body", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.17f, 0.17f), leg * XMMatrixTranslation(0, 0.91f - margin, -0.115f) },

			// 왼 종아리
			{ "LLeg2", "LLeg1", XMFLOAT3(0.21f - margin, 0, 0), XMFLOAT3(0.4f, 0.16f, 0.16f), leg * XMMatrixTranslation(0, 0.5f - margin, -0.1f) },

			// 왼발
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

		{
			auto l1 = GetWorldPosition("LLeg1");
			auto l2 = GetWorldPosition("LLeg2");
			auto l3 = GetWorldPosition("LFoot");

			legLen.x = Distance(l1, l2);
			legLen.y = Distance(l2, l3);
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
			throw invalid_argument("같은 이름의 본이 있습니다.");
		}

		int parIndex = -1;
		if (!parentName.empty())
		{
			auto it = nameToIndex.find(parentName);
			if (it == nameToIndex.end())
			{
				throw invalid_argument("주어진 부모 이름의 본을 찾을 수 없습니다.");
			}
			parIndex = it->second;
		}

		DWORD color = 0x0aa0a0ff;

		nameToIndex.insert(make_pair(name, (int)bodies.size()));

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
		// 로컬 트랜스폼이 없다고 가정하면
		// (내 링크) x (부모의 월드) = (내 월드)
		// (내 링크) = (내 월드) x (부모의 월드)^(-1)

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

			// (내 월드) = (내 로컬) x (내 링크) x (부모의 월드)
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
				for (size_t i = 0; i < mesh->Vertices().second; ++i)
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

	void Animate_Test(DWORD elapsed)
	{
		total += elapsed;

		float angle = total / 5000.0f * 2 * (float)M_PI;

		float headSwing = sin(angle) * (float)M_PI * 0.3f;
		float leg1Swing = sin(angle) * (float)M_PI * 0.3f;
		float leg2Swing = (1 - cos(angle)) / 2 * (float)M_PI / 2;
		float armSwing = (1 - cos(angle)) / 2 * (float)M_PI;
		float armSwing2 = (1 - cos(angle)) / 2 * (float)M_PI / 2;

		bodies[GetBoneIndex("Body")]->localTx = XMMatrixRotationZ(angle);
		bodies[GetBoneIndex("Head")]->localTx = XMMatrixRotationY(headSwing);
		bodies[GetBoneIndex("RArm1")]->localTx = XMMatrixRotationY(-armSwing2) * XMMatrixRotationZ(armSwing);
		bodies[GetBoneIndex("RArm2")]->localTx = XMMatrixRotationZ(leg2Swing);
		bodies[GetBoneIndex("LArm1")]->localTx = XMMatrixRotationY(armSwing2) * XMMatrixRotationZ(armSwing);
		bodies[GetBoneIndex("LArm2")]->localTx = XMMatrixRotationZ(leg2Swing);

		bodies[GetBoneIndex("RLeg1")]->localTx = XMMatrixRotationZ(leg1Swing);
		bodies[GetBoneIndex("RLeg2")]->localTx = XMMatrixRotationZ(-leg2Swing);
		bodies[GetBoneIndex("LLeg1")]->localTx = XMMatrixRotationZ(-leg1Swing);
		bodies[GetBoneIndex("LLeg2")]->localTx = XMMatrixRotationZ(-leg2Swing);
	}

	void Update()
	{
		UpdateWorldTransform();
		TransformMesh();
	}

	XMFLOAT3 GetWorldPosition(const string& name)
	{
		auto index = GetBoneIndex(name);
		if (index > 0)
		{
			auto& worldTx = bodies[index]->worldTx;
			return XMFLOAT3(
				worldTx.r[3].m128_f32[0],
				worldTx.r[3].m128_f32[1],
				worldTx.r[3].m128_f32[2]);
		}
		return XMFLOAT3(0, 0, 0);
	}

	static void SetTranslation(XMMATRIX& tx, const XMFLOAT3& pos)
	{
		tx.r[3].m128_f32[0] = pos.x;
		tx.r[3].m128_f32[1] = pos.y;
		tx.r[3].m128_f32[2] = pos.z;
	}

	static XMFLOAT3 GetTranslation(const XMMATRIX& tx)
	{
		return XMFLOAT3(tx.r[3].m128_f32[0], tx.r[3].m128_f32[1], tx.r[3].m128_f32[2]);
	}

	void SetFootPosition(bool left, const XMFLOAT3& pos_)
	{
		const auto& comTx = bodies[0]->worldTx;

		int index[] = 
		{
			GetBoneIndex(left ? "LLeg1" : "RLeg1"),
			GetBoneIndex(left ? "LLeg2" : "RLeg2"),
			GetBoneIndex(left ? "LFoot" : "RFoot")
		};

		XMFLOAT3 orgPos[] =
		{
			GetTranslation(bodies[index[0]]->worldTx),
			GetTranslation(bodies[index[1]]->worldTx),
			GetTranslation(bodies[index[2]]->worldTx),
		};

		XMFLOAT3 pos = pos_;// orgPos[2];

		XMFLOAT3 comAxis[] = 
		{
			FrameHelper::GetX(comTx),
			FrameHelper::GetY(comTx),
			FrameHelper::GetZ(comTx),
		};

		auto center = (orgPos[0] + pos) / 2;
		XMFLOAT3 x = Normalize(pos - orgPos[0]);

		// X축 방향을 부모 트랜스폼으로 보낸다
		XMFLOAT3 xInCom(
			Dot(comAxis[0], x),
			Dot(comAxis[1], x),
			Dot(comAxis[2], x));

		// IK로 발 방향을 구한다 (Y축 방향)
		auto footInCom = GetFootDirection(xInCom);

		XMFLOAT3 y =
			comAxis[0] * footInCom.x +
			comAxis[1] * footInCom.y +
			comAxis[2] * footInCom.z;

		XMFLOAT3 z = Cross(x, y);

		XMMATRIX legTx = XMMatrixIdentity();
		FrameHelper::SetX(legTx, x);
		FrameHelper::SetY(legTx, y);
		FrameHelper::SetZ(legTx, z);

		XMMATRIX footTx = XMMatrixIdentity();
		FrameHelper::SetX(footTx, y);
		FrameHelper::SetY(footTx, -x);
		FrameHelper::SetZ(footTx, z);

		{
			auto& worldTx = bodies[index[0]]->worldTx;

			worldTx = legTx;
			SetTranslation(worldTx, orgPos[0]);

			auto& localTx = bodies[index[0]]->localTx;
			auto parent = bodies[bodies[index[0]]->parentIndex];
			localTx = worldTx *
				XMMatrixInverse(nullptr, parent->worldTx) *
				XMMatrixInverse(nullptr, bodies[index[0]]->linkTx);
		}

		{
			auto& worldTx = bodies[index[1]]->worldTx;

			worldTx = legTx;
			SetTranslation(worldTx, center);

			auto& localTx = bodies[index[1]]->localTx;
			auto parent = bodies[bodies[index[1]]->parentIndex];
			localTx = worldTx *
				XMMatrixInverse(nullptr, parent->worldTx) *
				XMMatrixInverse(nullptr, bodies[index[1]]->linkTx);
		}

		{
			auto& worldTx = bodies[index[2]]->worldTx;

			worldTx = footTx;
			SetTranslation(worldTx, pos);

			auto& localTx = bodies[index[2]]->localTx;
			auto parent = bodies[bodies[index[2]]->parentIndex];
			localTx = worldTx *
				XMMatrixInverse(nullptr, parent->worldTx) *
				XMMatrixInverse(nullptr, bodies[index[2]]->linkTx);
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
	unique_ptr<IMesh> frame;
	vector<RobotBody*> bodies;
	map<string, int> nameToIndex;

	XMFLOAT2 legLen;

	DWORD total = 0;
};

IRobot* IRobot::Create()
{ return new Robot; }

XMFLOAT3 IRobot::GetFootDirection(const XMFLOAT3& legDir_)
{
	XMFLOAT3 legDir = Normalize(legDir_);

	// 좌표에서 다시 각도를 구해서 각도를 기준으로 처리하자
	// (실제로 IK 처리 시에는 좌표만 주어질 것이므로)
	float angleZ_R = acosf(-legDir.y);

	float angleY_R = abs(angleZ_R) > 0.0001f ? atan2f(
		legDir.z / -sinf(angleZ_R),
		legDir.x / sinf(angleZ_R)) : 0;

	XMFLOAT3 footDir(
		cosf(angleZ_R) * cosf(angleY_R),
		sinf(angleZ_R),
		-cosf(angleZ_R) * sinf(angleY_R));

	// 다리 방향이 뒤로 향하는 경우는 바깥이 아니라 몸 안 쪽을 보게 한다
	if (legDir.x < 0)
	{
		footDir = -footDir;
	}

	// Y축 회전이 +/-45~135도 범위일 때는 발 방향이 앞을 보게 한다
	float h1Factor = 0;
	if (angleY_R > AngleHelperF::DegreeToRadian(-135) &&
		angleY_R < AngleHelperF::DegreeToRadian(-45))
	{
		if (angleY_R > AngleHelperF::DegreeToRadian(-90) &&
			angleY_R < AngleHelperF::DegreeToRadian(-45))
		{
			// -45 ~ -90
			h1Factor = 1 - (90 + AngleHelperF::RadianToDegree(angleY_R)) / 45.0f;
		}
		else
		{
			// -90 ~ -135
			h1Factor = (135 + AngleHelperF::RadianToDegree(angleY_R)) / 45.0f;
		}
	}
	else if (
		angleY_R > AngleHelperF::DegreeToRadian(45) &&
		angleY_R < AngleHelperF::DegreeToRadian(135))
	{
		if (angleY_R > AngleHelperF::DegreeToRadian(45) &&
			angleY_R < AngleHelperF::DegreeToRadian(90))
		{
			// 45 ~ 90
			h1Factor = (AngleHelperF::RadianToDegree(angleY_R) - 45) / 45.0f;
		}
		else
		{
			// 90 ~ 135
			h1Factor = 1 - (AngleHelperF::RadianToDegree(angleY_R) - 90) / 45.0f;
		}
	}

	// Z축 회전이 90도를 넘어가면 그냥 원래 앞 방향을 보게 한다
	float h2Factor =
		angleZ_R > AngleHelperF::DegreeToRadian(90) ?
		(AngleHelperF::RadianToDegree(angleZ_R) - 90) / 90.0f :
		0;

	XMFLOAT3 footDirH;
	XMStoreFloat3(
		&footDirH,
		XMVectorLerp(
			XMVectorLerp(
				XMLoadFloat3(&footDir),
				XMLoadFloat3(&XMFLOAT3(1, 0, 0)),
				h1Factor),
			XMLoadFloat3(&footDir),
			h2Factor));
	footDirH = Normalize(footDirH);

	return footDirH;
}
