#include "pch.h"
#include "RobotBuilder.h"

#include "VectorDXMathAdaptor.h"

#include "RobotImplementation.h"

using namespace std;
using namespace DirectX;
using namespace Core;

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

	// ¸öÅë
	{ "Body", string(), Vector3D(0, 0, 0), Vector3D(0.25f, 0.61f, 0.4f), DXMathTransform<double>::Translation(0, 1.22f, 0) },

	// ¸Ó¸®
	{ "Head", "Body", Vector3D(0.0, 0.125, 0.0), Vector3D(0.22, 0.25, 0.22), DXMathTransform<double>::Translation(0, 1.54f, 0) },

	// ¿À¸¥À­ÆÈ
	{ "RArm1", "Body", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35, 0.11, 0.11), g_legRot * DXMathTransform<double>::Translation(0.0, 1.53f - g_margin, 0.265f) },

	// ¿À¸¥ ¾Æ·§ÆÈ
	{ "RArm2", "RArm1", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35f, 0.11f, 0.11f), g_legRot * DXMathTransform<double>::Translation(0.0, 1.17f - g_margin, 0.265f) },

	// ¿ÞÀ­ÆÈ
	{ "LArm1", "Body", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35f, 0.11f, 0.11f), g_legRot * DXMathTransform<double>::Translation(0.0, 1.53f - g_margin, -0.265f) },

	// ¿Þ¾Æ·§ÆÈ
	{ "LArm2", "LArm1", Vector3D(0.175 - g_margin, 0.0, 0.0), Vector3D(0.35f, 0.11f, 0.11f), g_legRot * DXMathTransform<double>::Translation(0.0, 1.17f - g_margin, -0.265f) },

	// ¿À¸¥ Çã¹÷Áö
	{ "RLeg1", "Body", Vector3D(0.21 - g_margin, 0.0, 0.0), Vector3D(0.4, 0.17, 0.17), g_legRot * DXMathTransform<double>::Translation(0.0, 0.91f - g_margin, 0.115f) },

	// ¿À¸¥ Á¾¾Æ¸®
	{ "RLeg2", "RLeg1", Vector3D(0.21f - g_margin, 0.0, 0.0), Vector3D(0.4f, 0.16f, 0.16f), g_legRot * DXMathTransform<double>::Translation(0.0, 0.5f - g_margin, 0.1f) },

	// ¿À¸¥ ¹ß
	{ "RFoot", "RLeg2", Vector3D(0.05, 0.0, 0.0), Vector3D(0.25, 0.09, 0.16), DXMathTransform<double>::Translation(0.00f, 0.045f, 0.1f) },

	// ¿Þ Çã¹÷Áö
	{ "LLeg1", "Body", Vector3D(0.21f - g_margin, 0.0, 0.0), Vector3D(0.4f, 0.17f, 0.17f), g_legRot * DXMathTransform<double>::Translation(0.0, 0.91f - g_margin, -0.115f) },

	// ¿Þ Á¾¾Æ¸®
	{ "LLeg2", "LLeg1", Vector3D(0.21f - g_margin, 0.0, 0.0), Vector3D(0.4f, 0.16f, 0.16f), g_legRot * DXMathTransform<double>::Translation(0.0, 0.5f - g_margin, -0.1f) },

	// ¿Þ¹ß
	{ "LFoot", "LLeg2", Vector3D(0.05, 0.0, 0.0), Vector3D(0.25, 0.09, 0.16), DXMathTransform<double>::Translation(0.00f, 0.045f, -0.1f) },
};

//------------------------------------------------------------------------------
RobotBuilder::RobotBuilder(Robot* robot)
	: robot(robot)
{
	// È­»ìÇ¥´Â ÇÏ³ª¸¸ ¾´´Ù
	{
		float len = 0.3f;
		float headLen = 0.1f;
		float r1 = 0.0125f / sqrtf(2);
		float r2 = 0.025f / sqrtf(2);
		robot->frame.reset(ICoordinateAxisMesh::Create(
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
		auto l1 = robot->GetWorldPosition("LLeg1");
		auto l2 = robot->GetWorldPosition("LLeg2");
		auto l3 = robot->GetWorldPosition("LFoot");

		robot->legLen.x = Distance(l1, l2);
		robot->legLen.y = Distance(l2, l3);
	}

	for (size_t i = 0; i < robot->bodies.size(); ++i)
	{
		auto body = robot->bodies[i];
		body->CalculateLinkTransform();
	}

	robot->TransformMesh();
}

//------------------------------------------------------------------------------
void RobotBuilder::CreateBody(
	const string& name,
	const string& parentName,
	const Vector3D& pos,
	const Vector3D& size,
	const Matrix4D& worldTx)
{
	if (robot->nameToIndex.find(name) != robot->nameToIndex.end())
	{
		throw invalid_argument("°°Àº ÀÌ¸§ÀÇ º»ÀÌ ÀÖ½À´Ï´Ù.");
	}

	int parIndex = -1;
	if (!parentName.empty())
	{
		auto it = robot->nameToIndex.find(parentName);
		if (it == robot->nameToIndex.end())
		{
			throw invalid_argument("ÁÖ¾îÁø ºÎ¸ð ÀÌ¸§ÀÇ º»À» Ã£À» ¼ö ¾ø½À´Ï´Ù.");
		}
		parIndex = it->second;
	}

	DWORD color = 0x0aa0a0ff;

	robot->nameToIndex.insert(make_pair(name, (int)robot->bodies.size()));

	auto parent = parIndex >= 0 ? robot->bodies[parIndex] : nullptr;

	auto body = new RobotBody;
	body->name = name;
	body->parentIndex = parIndex;
	body->parent = parent;
	body->mesh.reset(IBoxMesh::Create(ToXMFLOAT3(pos), ToXMFLOAT3(size), color));
	body->SetWorldTx(worldTx);

	robot->bodies.push_back(body);
	robot->Append(body->mesh.get());
	robot->Append(robot->frame.get());

	if (parent != nullptr)
	{
		parent->children.push_back(body);
	}
}
