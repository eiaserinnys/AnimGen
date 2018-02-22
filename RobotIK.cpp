#include "pch.h"
#include "RobotIK.h"

#include "ExponentialMap.h"
#include "FrameHelper.h"
#include "AngleHelper.h"

#include "RobotImplementation.h"

using namespace std;
using namespace Core;
using namespace DirectX;

//------------------------------------------------------------------------------
RobotIK::RobotIK(Robot* robot)
	: robot(robot)
{
}

//------------------------------------------------------------------------------
Vector3D RobotIK::GetFootDirection(const Vector3D& legDir_)
{
	Vector3D legDir = Normalize(legDir_);

	// 좌표에서 다시 각도를 구해서 각도를 기준으로 처리하자
	// (실제로 IK 처리 시에는 좌표만 주어질 것이므로)
	double angleZ_R = acos(-legDir.y);

	double angleY_R = abs(angleZ_R) > 0.0001f ? atan2(
		legDir.z / -sin(angleZ_R),
		legDir.x / sin(angleZ_R)) : 0;

	Vector3D footDir(
		cos(angleZ_R) * cos(angleY_R),
		sin(angleZ_R),
		-cos(angleZ_R) * sin(angleY_R));

	// 다리 방향이 뒤로 향하는 경우는 바깥이 아니라 몸 안 쪽을 보게 한다
	if (legDir.x < 0 && abs(angleZ_R) > 0.0001f)
	{
		footDir = -footDir;
	}

	// Y축 회전이 +/-45~135도 범위일 때는 발 방향이 앞을 보게 한다
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

	// Z축 회전이 90도를 넘어가면 그냥 원래 앞 방향을 보게 한다
	double h2Factor =
		angleZ_R > AngleHelperD::DegreeToRadian(90) ?
		(AngleHelperD::RadianToDegree(angleZ_R) - 90) / 90.0 :
		0;

	Vector3D footDirH =
		Lerp(
			Lerp(footDir, Vector3D(1, 0, 0), h1Factor),
			footDir,
			h2Factor);
	footDirH = Normalize(footDirH);

	return footDirH;
}

//--------------------------------------------------------------------------
void RobotIK::SetFootPosition(bool left, const Vector3D& pos_)
{
	const auto& comTx = robot->bodies[0]->WorldTx();

	int index[] =
	{
		robot->GetBoneIndex(left ? "LLeg1" : "RLeg1"),
		robot->GetBoneIndex(left ? "LLeg2" : "RLeg2"),
		robot->GetBoneIndex(left ? "LFoot" : "RFoot")
	};

	Vector3D orgPos[] =
	{
		FrameHelper::GetTranslation(robot->bodies[index[0]]->WorldTx()),
		FrameHelper::GetTranslation(robot->bodies[index[1]]->WorldTx()),
		FrameHelper::GetTranslation(robot->bodies[index[2]]->WorldTx()),
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

	// X축 방향을 부모 트랜스폼으로 보낸다
	Vector3D xInCom(Dot(comAxis[0], x), Dot(comAxis[1], x), Dot(comAxis[2], x));

	// IK로 발 방향을 구한다 (Y축 방향)
	auto footInCom = GetFootDirection(xInCom);

	Vector3D y = comAxis[0] * footInCom.x + comAxis[1] * footInCom.y + comAxis[2] * footInCom.z;

	Vector3D z = Normalize(Cross(x, y));
	y = Normalize(Cross(z, x));

	auto orgLegLen = robot->legLen.x + robot->legLen.y;
	if (newLegLen > orgLegLen)
	{
		Vector3D kneePos = 
			orgPos[0] + 
			x * newLegLen * (robot->legLen.x / (robot->legLen.x + robot->legLen.y));

		{
			Matrix4D worldTx = Matrix4D::Identity();
			FrameHelper::Set(worldTx, x, y, z);
			FrameHelper::SetTranslation(worldTx, orgPos[0]);
			robot->bodies[index[0]]->SetWorldTx(worldTx);
		}

		{
			Matrix4D worldTx = Matrix4D::Identity();
			FrameHelper::Set(worldTx, x, y, z);
			FrameHelper::SetTranslation(worldTx, kneePos);
			robot->bodies[index[1]]->SetWorldTx(worldTx);
		}
	}
	else
	{
		// 굽힌 케이스
		// http://mathworld.wolfram.com/Sphere-SphereIntersection.html
		double d = newLegLen;
		double R = robot->legLen.x;
		double r = robot->legLen.y;
		double d1 = (d * d - r * r + R * R) / (2 * d);
		double a = 1 / d * std::sqrt((-d + r - R) * (-d - r + R) * (-d + r + R) *(d + r + R));

		// 새 무릎 위치
		Vector3D kneePos = x * d1 + y * (a / 2) + orgPos[0];

		{
			Vector3D x1 = Normalize(kneePos - orgPos[0]);
			Vector3D z1 = Normalize(Cross(x1, y));
			Vector3D y1 = Normalize(Cross(z1, x1));

			Matrix4D worldTx = Matrix4D::Identity();
			FrameHelper::Set(worldTx, x1, y1, z1);
			FrameHelper::SetTranslation(worldTx, orgPos[0]);
			robot->bodies[index[0]]->SetWorldTx(worldTx);
		}

		{
			Vector3D x1 = Normalize(pos - kneePos);
			Vector3D z1 = Normalize(Cross(x1, y));
			Vector3D y1 = Normalize(Cross(z1, x1));

			Matrix4D worldTx = Matrix4D::Identity();
			FrameHelper::Set(worldTx, x1, y1, z1);
			FrameHelper::SetTranslation(worldTx, kneePos);
			robot->bodies[index[1]]->SetWorldTx(worldTx);

			robot->bodies[index[1]]->LocalTx();
		}
	}

	{
		Matrix4D worldTx = Matrix4D::Identity();
		FrameHelper::Set(worldTx, y, (Vector3D)-x, z);
		FrameHelper::SetTranslation(worldTx, pos);
		robot->bodies[index[2]]->SetWorldTx(worldTx);
	}
}

//--------------------------------------------------------------------------
void RobotIK::SetFootTransform(
	bool left,
	const Vector3D& pos_,
	const Vector3D& rot_)
{
	SetFootPosition(left, pos_);

	auto m = ExponentialMap::ToMatrix(rot_);

	FrameHelper::SetTranslation(m, pos_);

	int index = robot->GetBoneIndex(left ? "LFoot" : "RFoot");

	robot->bodies[index]->SetWorldTx(m);
}
