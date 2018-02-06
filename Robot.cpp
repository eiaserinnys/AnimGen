#include "pch.h"
#include "Robot.h"

#include "FrameHelper.h"
#include "AngleHelper.h"

#include "Mesh.h"
#include "MeshT.h"
#include "ObjectBuffer.h"

#include "Vector.h"
#include "VectorDXMathAdaptor.h"
#include "Matrix.h"
#include "exp-map.h"

using namespace std;
using namespace Core;
using namespace DirectX;

struct RobotBody
{
	string name;
	int parentIndex;
	Matrix4D linkTx = Matrix4D::Identity();

	Matrix4D localTx = Matrix4D::Identity();
	Vector4D quat = Vector4D(0, 0, 0, 0);
	double expMap[3] = { 0, 0, 0 };
	Vector4D quatVerify = Vector4D(0, 0, 0, 0);

	Matrix4D worldTx = Matrix4D::Identity();
	unique_ptr<IMesh> mesh;
};

struct BodyDesc
{
	string name;
	string parentName;
	Vector3D position;
	Vector3D extent;
	Matrix4D worldTx;
};

static const double g_margin = 0.05;
static const Matrix4D g_legRot = DXMathTransform<double>::RotationZ(-90.0f / 180 * (float)M_PI);

static BodyDesc desc[] =
{

	// ����
	{ "Body", string(), Vector3D(0, 0, 0), Vector3D(0.25f, 0.61f, 0.4f), DXMathTransform<double>::Translation(0, 1.22f, 0) },

	// �Ӹ�
	{ "Head", "Body", Vector3D(0.0, 0.125, 0.0), Vector3D(0.22, 0.25, 0.22), DXMathTransform<double>::Translation(0, 1.54f, 0) },

	// ��������
	{ "RArm1", "Body", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35, 0.11, 0.11), g_legRot * DXMathTransform<double>::Translation(0.0, 1.53f - g_margin, 0.265f) },

	// ���� �Ʒ���
	{ "RArm2", "RArm1", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35f, 0.11f, 0.11f), g_legRot * DXMathTransform<double>::Translation(0.0, 1.17f - g_margin, 0.265f) },

	// ������
	{ "LArm1", "Body", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35f, 0.11f, 0.11f), g_legRot * DXMathTransform<double>::Translation(0.0, 1.53f - g_margin, -0.265f) },

	// �޾Ʒ���
	{ "LArm2", "LArm1", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35f, 0.11f, 0.11f), g_legRot * DXMathTransform<double>::Translation(0.0, 1.17f - g_margin, -0.265f) },

	// ���� �����
	{ "RLeg1", "Body", Vector3D(0.21 - g_margin, 0.0, 0.0), Vector3D(0.4, 0.17, 0.17), g_legRot * DXMathTransform<double>::Translation(0.0, 0.91f - g_margin, 0.115f) },

	// ���� ���Ƹ�
	{ "RLeg2", "RLeg1", Vector3D(0.21f - g_margin, 0.0, 0.0), Vector3D(0.4f, 0.16f, 0.16f), g_legRot * DXMathTransform<double>::Translation(0.0, 0.5f - g_margin, 0.1f) },

	// ���� ��
	{ "RFoot", "RLeg2", Vector3D(0.05, 0.0, 0.0), Vector3D(0.25, 0.09, 0.16), DXMathTransform<double>::Translation(0.00f, 0.045f, 0.1f) },

	// �� �����
	{ "LLeg1", "Body", Vector3D(0.21f - g_margin, 0.0, 0.0), Vector3D(0.4f, 0.17f, 0.17f), g_legRot * DXMathTransform<double>::Translation(0.0, 0.91f - g_margin, -0.115f) },

	// �� ���Ƹ�
	{ "LLeg2", "LLeg1", Vector3D(0.21f - g_margin, 0.0, 0.0), Vector3D(0.4f, 0.16f, 0.16f), g_legRot * DXMathTransform<double>::Translation(0.0, 0.5f - g_margin, -0.1f) },

	// �޹�
	{ "LFoot", "LLeg2", Vector3D(0.05, 0.0, 0.0), Vector3D(0.25, 0.09, 0.16), DXMathTransform<double>::Translation(0.00f, 0.045f, -0.1f) },
};

class Robot : public MeshT<IRobot> {
public:
	//--------------------------------------------------------------------------
	Robot()
	{
		// ȭ��ǥ�� �ϳ��� ����
		{
			float len = 0.3f;
			float headLen = 0.1f;
			float r1 = 0.0125f / sqrtf(2);
			float r2 = 0.025f / sqrtf(2);
			frame.reset(ICoordinateAxisMesh::Create(
				XMMatrixIdentity(), len, headLen, r1, r2, 8));
		}

		DWORD color = 0x0aa0a0ff;

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

	//--------------------------------------------------------------------------
	void CreateBody(
		const string& name, 
		const string& parentName,
		const Vector3D& pos, 
		const Vector3D& size,
		const Matrix4D& worldTx)
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

		nameToIndex.insert(make_pair(name, (int)bodies.size()));

		auto body = new RobotBody;
		body->name = name;
		body->parentIndex = parIndex;
		body->mesh.reset(IBoxMesh::Create(ToXMFLOAT3(pos), ToXMFLOAT3(size), color));
		body->worldTx = worldTx;

		bodies.push_back(body);
		Append(body->mesh.get());
		Append(frame.get());
	}

	//--------------------------------------------------------------------------
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
					bodies[body->parentIndex]->worldTx.Inverse();
			}
			else
			{
				body->linkTx = body->worldTx;
			}
		}
	}

	//--------------------------------------------------------------------------
	void CalculateLocalRotation()
	{
		for (auto it = bodies.begin(); it != bodies.end(); ++it)
		{
			auto body = *it;

			CalculateLocalRotation(body);
		}
	}

	//--------------------------------------------------------------------------
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

	//--------------------------------------------------------------------------
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
					pos[offset] =
						ToXMFLOAT3(DXMathTransform<double>::Transform(
							FromXMFLOAT3(mesh->Vertices().first[i]),
							body->worldTx));
					nor[offset] = 
						ToXMFLOAT3(DXMathTransform<double>::TransformNormal(
							FromXMFLOAT3(mesh->Normals().first[i]),
							body->worldTx));

					offset++;
				}
			};

			transform(body->mesh.get());
			transform(frame.get());
		}
	}

	//--------------------------------------------------------------------------
	int GetBoneIndex(const string& name) const
	{
		auto it = nameToIndex.find(name);
		return it != nameToIndex.end() ? it->second : -1;
	}

	//--------------------------------------------------------------------------
	void Animate_Test(DWORD elapsed)
	{
		total += elapsed;

		double angle = total / 5000.0f * 2 * M_PI;

		double headSwing = sin(angle) * M_PI * 0.3;
		double leg1Swing = sin(angle) * M_PI * 0.3;
		double leg2Swing = (1 - cos(angle)) / 2 * M_PI / 2;
		double armSwing = (1 - cos(angle)) / 2 * M_PI;
		double armSwing2 = (1 - cos(angle)) / 2 * M_PI / 2;

		bodies[GetBoneIndex("Body")]->localTx = DXMathTransform<double>::RotationZ(angle);
		bodies[GetBoneIndex("Head")]->localTx = DXMathTransform<double>::RotationY(headSwing);
		bodies[GetBoneIndex("RArm1")]->localTx = DXMathTransform<double>::RotationY(-armSwing2) * DXMathTransform<double>::RotationZ(armSwing);
		bodies[GetBoneIndex("RArm2")]->localTx = DXMathTransform<double>::RotationZ(leg2Swing);
		bodies[GetBoneIndex("LArm1")]->localTx = DXMathTransform<double>::RotationY(armSwing2) * DXMathTransform<double>::RotationZ(armSwing);
		bodies[GetBoneIndex("LArm2")]->localTx = DXMathTransform<double>::RotationZ(leg2Swing);

		bodies[GetBoneIndex("RLeg1")]->localTx = DXMathTransform<double>::RotationZ(leg1Swing);
		bodies[GetBoneIndex("RLeg2")]->localTx = DXMathTransform<double>::RotationZ(-leg2Swing);
		bodies[GetBoneIndex("LLeg1")]->localTx = DXMathTransform<double>::RotationZ(-leg1Swing);
		bodies[GetBoneIndex("LLeg2")]->localTx = DXMathTransform<double>::RotationZ(-leg2Swing);
	}

	//--------------------------------------------------------------------------
	void Update()
	{
		CalculateLocalRotation();
		UpdateWorldTransform();
		TransformMesh();
	}

	//--------------------------------------------------------------------------
	RobotBody* Find(const string& name)
	{
		auto index = GetBoneIndex(name);
		if (index > 0) { return bodies[index]; }
		return nullptr;
	}

	//--------------------------------------------------------------------------
	const RobotBody* Find(const string& name) const
	{
		return const_cast<Robot*>(this)->Find(name);
	}

	//--------------------------------------------------------------------------
	Vector3D GetWorldPosition(const string& name)
	{
		auto index = GetBoneIndex(name);
		if (index > 0)
		{
			auto& worldTx = bodies[index]->worldTx;
			return FrameHelper::GetTranslation(worldTx);
		}
		return Vector3D(0, 0, 0);
	}

	//--------------------------------------------------------------------------
	void SetFootPosition(bool left, const Vector3D& pos_)
	{
		const auto& comTx = bodies[0]->worldTx;

		int index[] = 
		{
			GetBoneIndex(left ? "LLeg1" : "RLeg1"),
			GetBoneIndex(left ? "LLeg2" : "RLeg2"),
			GetBoneIndex(left ? "LFoot" : "RFoot")
		};

		Vector3D orgPos[] =
		{
			FrameHelper::GetTranslation(bodies[index[0]]->worldTx),
			FrameHelper::GetTranslation(bodies[index[1]]->worldTx),
			FrameHelper::GetTranslation(bodies[index[2]]->worldTx),
		};

		Vector3D pos = pos_;// orgPos[2];

		Vector3D comAxis[] =
		{
			FrameHelper::GetX(comTx),
			FrameHelper::GetY(comTx),
			FrameHelper::GetZ(comTx),
		};

		Vector3D x = pos - orgPos[0];
		double newLegLen = Length(x);
		x = Normalize(x);

		// X�� ������ �θ� Ʈ���������� ������
		Vector3D xInCom(Dot(comAxis[0], x), Dot(comAxis[1], x), Dot(comAxis[2], x));

		// IK�� �� ������ ���Ѵ� (Y�� ����)
		auto footInCom = GetFootDirection(xInCom);

		Vector3D y = comAxis[0] * footInCom.x + comAxis[1] * footInCom.y + comAxis[2] * footInCom.z;

		Vector3D z = Normalize(Cross(x, y));
		y = Normalize(Cross(z, x));

		if (newLegLen > legLen.x + legLen.y)
		{
			Vector3D kneePos = (orgPos[0] + pos) * (legLen.x / (legLen.x + legLen.y));

			{
				auto& worldTx = bodies[index[0]]->worldTx;
				FrameHelper::Set(worldTx, x, y, z);
				FrameHelper::SetTranslation(worldTx, orgPos[0]);
			}

			{
				auto& worldTx = bodies[index[1]]->worldTx;
				FrameHelper::Set(worldTx, x, y, z);
				FrameHelper::SetTranslation(worldTx, kneePos);
			}
		}
		else
		{
			// ���� ���̽�
			// http://mathworld.wolfram.com/Sphere-SphereIntersection.html
			double d = newLegLen;
			double R = legLen.x;
			double r = legLen.y;
			double d1 = (d * d - r * r + R * R) / (2 * d);
			double a = 1 / d * sqrtf((-d + r - R) * (-d - r + R) * (-d + r + R) *(d + r + R));

			// �� ���� ��ġ
			Vector3D kneePos = x * d1 + y * (a / 2) + orgPos[0];

			{
				Vector3D x1 = Normalize(kneePos - orgPos[0]);
				Vector3D z1 = Normalize(Cross(x1, y));
				Vector3D y1 = Normalize(Cross(z1, x1));

				auto& worldTx = bodies[index[0]]->worldTx;
				FrameHelper::Set(worldTx, x1, y1, z1);
				FrameHelper::SetTranslation(worldTx, orgPos[0]);
			}

			{
				Vector3D x1 = Normalize(orgPos[2] - kneePos);
				Vector3D z1 = Normalize(Cross(x1, y));
				Vector3D y1 = Normalize(Cross(z1, x1));

				auto& worldTx = bodies[index[1]]->worldTx;
				FrameHelper::Set(worldTx, x1, y1, z1);
				FrameHelper::SetTranslation(worldTx, kneePos);
			}
		}

		{
			auto& worldTx = bodies[index[2]]->worldTx;
			FrameHelper::Set(worldTx, y, (Vector3D)-x, z);
			FrameHelper::SetTranslation(worldTx, pos);
		}

		CalculateLocalTransform(index[0]);
		CalculateLocalTransform(index[1]);
		CalculateLocalTransform(index[2]);
	}

	//--------------------------------------------------------------------------
	~Robot()
	{
		for (auto it = bodies.begin(); it != bodies.end(); ++it)
		{
			delete *it;
		}
		bodies.clear();
	}

	//--------------------------------------------------------------------------
	void CalculateLocalTransform(int index)
	{
		auto& localTx = bodies[index]->localTx;

		if (bodies[index]->parentIndex >= 0)
		{
			auto parent = bodies[bodies[index]->parentIndex];
			localTx = bodies[index]->worldTx *
				parent->worldTx.Inverse() *
				bodies[index]->linkTx.Inverse();
		}
		else
		{
			localTx = bodies[index]->worldTx *
				bodies[index]->linkTx.Inverse();
		}
	}

	//--------------------------------------------------------------------------
	// �ϴ� ��, �ٸ�, �߸� ����Ѵ�
	void CalculateGeneralCoordinate()
	{
		coord.bodyPos = GetWorldPosition("Body");
		//coord.bodyROt = GetLocalRotation("Body");
	}

	//--------------------------------------------------------------------------
	const double* GetLocalRotation(const string& name)
	{
		// ���� ���ʹϾ����� �ٲ� ��
		auto found = Find(name);
		if (found != nullptr) { return found->expMap; }

		static double identity[] = { 0, 0, 0 };
		return identity;
	}

	//--------------------------------------------------------------------------
	const Vector4D GetLocalQuaternion(const string& name)
	{
		// ���� ���ʹϾ����� �ٲ� ��
		auto found = Find(name);
		if (found != nullptr) { return found->quat; }
		return Vector4D(0, 0, 0, 1);
	}

	//--------------------------------------------------------------------------
	const Vector4D GetLocalQuaternionVerify(const string& name)
	{
		// ���� ���ʹϾ����� �ٲ� ��
		auto found = Find(name);
		if (found != nullptr) { return found->quatVerify; }
		return Vector4D(0, 0, 0, 1);
	}

	//--------------------------------------------------------------------------
	void CalculateLocalRotation(RobotBody* found)
	{
		static const double epsilon = sqrt(sqrt(0.000001f));

		auto& quat = found->quat;

		quat = Normalize(DXMathTransform<double>::QuaternionRotationMatrix(found->localTx));

		double theta = acos(quat.w) * 2;

		double sinHalfTheta = sin(theta / 2);

		double m = 0;

		if (abs(theta) < epsilon)
		{
			// ���Ϸ� ������ ���ؼ�
			m = 1 / (1.0 / 2 + theta * theta / 48);
		}
		else
		{
			m = theta / sinHalfTheta;
		}

		found->expMap[0] = m * quat.x;
		found->expMap[1] = m * quat.y;
		found->expMap[2] = m * quat.z;

		double quatV[4] = { 0, 0, 0, 0 };

		EM_To_Q(found->expMap, quatV, 0);

		found->quatVerify.x = quatV[0];
		found->quatVerify.y = quatV[1];
		found->quatVerify.z = quatV[2];
		found->quatVerify.w = quatV[3];
	}

	virtual const GeneralCoordinate& Current() const { return coord; }

protected:
	unique_ptr<IMesh> frame;
	vector<RobotBody*> bodies;
	map<string, int> nameToIndex;

	GeneralCoordinate coord;

	Vector2D legLen;

	DWORD total = 0;
};

//------------------------------------------------------------------------------
IRobot* IRobot::Create()
{ return new Robot; }

//------------------------------------------------------------------------------
Vector3D IRobot::GetFootDirection(const Vector3D& legDir_)
{
	Vector3D legDir = Normalize(legDir_);

	// ��ǥ���� �ٽ� ������ ���ؼ� ������ �������� ó������
	// (������ IK ó�� �ÿ��� ��ǥ�� �־��� ���̹Ƿ�)
	double angleZ_R = acos(-legDir.y);

	double angleY_R = abs(angleZ_R) > 0.0001f ? atan2(
		legDir.z / -sin(angleZ_R),
		legDir.x / sin(angleZ_R)) : 0;

	Vector3D footDir(
		cos(angleZ_R) * cos(angleY_R),
		sin(angleZ_R),
		-cos(angleZ_R) * sin(angleY_R));

	// �ٸ� ������ �ڷ� ���ϴ� ���� �ٱ��� �ƴ϶� �� �� ���� ���� �Ѵ�
	if (legDir.x < 0)
	{
		footDir = -footDir;
	}

	// Y�� ȸ���� +/-45~135�� ������ ���� �� ������ ���� ���� �Ѵ�
	double h1Factor = 0;
	if (angleY_R > AngleHelperD::DegreeToRadian(-135) &&
		angleY_R < AngleHelperD::DegreeToRadian(-45))
	{
		if (angleY_R > AngleHelperD::DegreeToRadian(-90) &&
			angleY_R < AngleHelperD::DegreeToRadian(-45))
		{
			// -45 ~ -90
			h1Factor = 1 - (90 + AngleHelperD::RadianToDegree(angleY_R)) / 45.0;
		}
		else
		{
			// -90 ~ -135
			h1Factor = (135 + AngleHelperD::RadianToDegree(angleY_R)) / 45.0;
		}
	}
	else if (
		angleY_R > AngleHelperD::DegreeToRadian(45) &&
		angleY_R < AngleHelperD::DegreeToRadian(135))
	{
		if (angleY_R > AngleHelperD::DegreeToRadian(45) &&
			angleY_R < AngleHelperD::DegreeToRadian(90))
		{
			// 45 ~ 90
			h1Factor = (AngleHelperD::RadianToDegree(angleY_R) - 45) / 45.0;
		}
		else
		{
			// 90 ~ 135
			h1Factor = 1 - (AngleHelperD::RadianToDegree(angleY_R) - 90) / 45.0;
		}
	}

	// Z�� ȸ���� 90���� �Ѿ�� �׳� ���� �� ������ ���� �Ѵ�
	double h2Factor =
		angleZ_R > AngleHelperD::DegreeToRadian(90) ?
		(AngleHelperD::RadianToDegree(angleZ_R) - 90) / 90.0 :
		0;

	Vector3D footDirH =
		Lerp(
			Lerp(footDir, Vector3D(1, 0 ,0), h1Factor),
			footDir,
			h2Factor);
	footDirH = Normalize(footDirH);

	return footDirH;
}
