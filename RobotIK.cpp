#include "pch.h"
#include "RobotIK.h"

#include <WindowsUtility.h>

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
Vector3D RobotIK::GetFootDirection(const Vector3D& legDir_, bool left)
{
	Vector3D legDir = Normalize(legDir_);

	// 좌표에서 다시 각도를 구해서 각도를 기준으로 처리하자
	// (실제로 IK 처리 시에는 좌표만 주어질 것이므로)
	double angleZ_R = acos(-legDir.y);

	double angleY_R = std::atan2(- legDir.z, legDir.x);

	Vector3D footDir(
		std::cos(angleZ_R) * std::cos(angleY_R),
		std::sin(angleZ_R),
		-std::cos(angleZ_R) * std::sin(angleY_R));

	Vector3D footDir2(
		- legDir.y * std::cos(angleY_R),
		std::sin(angleZ_R),
		legDir.y * std::sin(angleY_R));

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

	if (angleZ_R < AngleHelperD::DegreeToRadian(5))
	{
		// 다리가 아래로 향한 각도가 5도 이내인 경우 그냥 발 앞 방향을 보게 한다
		// 약간의 움직임으로 발끝이 너무 크게 움직이는 오차를 줄이기 위한 처리
		double factor = angleZ_R / AngleHelperD::DegreeToRadian(5);

		Vector3D footDirF = Lerp(
			Vector3D(1, 0, 0),
			footDirH,
			factor);

		return Normalize(footDirF);
	}
	else
	{
		return footDirH;
	}
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
	auto footInCom = GetFootDirection(xInCom, left);

	Vector3D y = comAxis[0] * footInCom.x + comAxis[1] * footInCom.y + comAxis[2] * footInCom.z;

	Vector3D z = Normalize(Cross(x, y));
	y = Normalize(Cross(z, x));


	auto orgLegLen = robot->legLen.x + robot->legLen.y;

	bool stretching = newLegLen > orgLegLen;

	if (IsDumpEnabled(left))
	{
		WindowsUtility::Debug(stretching ? L"No Bending\n" : L"Bending\n");
		WindowsUtility::Debug(L"1\t%+.10f\t%+.10f\t%+.10f\n", orgPos[0].x, orgPos[0].y, orgPos[0].z);
		WindowsUtility::Debug(L"2\t%+.10f\t%+.10f\t%+.10f\n", orgPos[1].x, orgPos[1].y, orgPos[1].z);
		WindowsUtility::Debug(L"3\t%+.10f\t%+.10f\t%+.10f\n", orgPos[2].x, orgPos[2].y, orgPos[2].z);

		WindowsUtility::Debug(L"X\t%+.10f\t%+.10f\t%+.10f\n", x.x, x.y, x.z);
		WindowsUtility::Debug(L"Y\t%+.10f\t%+.10f\t%+.10f\n", y.x, y.y, y.z);
		WindowsUtility::Debug(L"Z\t%+.10f\t%+.10f\t%+.10f\n", z.x, z.y, z.z);
		WindowsUtility::Debug(L"F\t%+.10f\t%+.10f\t%+.10f\n", footInCom.x, footInCom.y, footInCom.z);
	}

	if (stretching)
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
		double delta = orgLegLen - newLegLen;
		double f = Utility::ClampGreaterThanOrEqualTo(delta, 0.01) / 0.01;

		// 굽힌 케이스
		// http://mathworld.wolfram.com/Sphere-SphereIntersection.html
		double d = newLegLen;
		double R = robot->legLen.x;
		double r = robot->legLen.y;
		double d1 = (d * d - r * r + R * R) / (2 * d);
		double a = 1 / d * std::sqrt((-d + r - R) * (-d - r + R) * (-d + r + R) *(d + r + R));

		// 새 무릎 위치
		Vector3D kneePos = x * d1 + y * (a / 2) + orgPos[0];

		if (IsDumpEnabled(left))
		{
			//WindowsUtility::Debug(
			//	L"S\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
			//	a, d, d1, (-d + r - R), (-d - r + R), (-d + r + R), (d + r + R));

			WindowsUtility::Debug(
				L"K\t%+.10f\t%+.10f\t%+.10f\n",
				kneePos.x, kneePos.y, kneePos.z);
		}

		Vector3D midKneePos =
			orgPos[0] +
			x * newLegLen * (robot->legLen.x / (robot->legLen.x + robot->legLen.y));

		Vector3D lk = Lerp(midKneePos, kneePos, f);

		{
			Vector3D x1 = Normalize(kneePos - orgPos[0]);
			Vector3D z1 = Normalize(Cross(x1, y));
			Vector3D y1 = Normalize(Cross(z1, x1));

			Vector3D x2 = Normalize(Lerp(x, x1, f));
			Vector3D y2 = Normalize(Lerp(y, y1, f));
			Vector3D z2 = Normalize(Lerp(z, z1, f));

			Matrix4D worldTx = Matrix4D::Identity();
			FrameHelper::Set(worldTx, x2, y2, z2);
			FrameHelper::SetTranslation(worldTx, orgPos[0]);
			robot->bodies[index[0]]->SetWorldTx(worldTx);

			if (IsDumpEnabled(left))
			{
				WindowsUtility::Debug(L"X1\t%+.10f\t%+.10f\t%+.10f\n", x1.x, x1.y, x1.z);
				WindowsUtility::Debug(L"Y1\t%+.10f\t%+.10f\t%+.10f\n", y1.x, y1.y, y1.z);
				WindowsUtility::Debug(L"Z1\t%+.10f\t%+.10f\t%+.10f\n", z1.x, z1.y, z1.z);
			}
		}

		{
			Vector3D x1 = Normalize(pos - kneePos);
			Vector3D z1 = Normalize(Cross(x1, y));
			Vector3D y1 = Normalize(Cross(z1, x1));

			Vector3D x2 = Normalize(Lerp(x, x1, f));
			Vector3D y2 = Normalize(Lerp(y, y1, f));
			Vector3D z2 = Normalize(Lerp(z, z1, f));

			Matrix4D worldTx = Matrix4D::Identity();
			FrameHelper::Set(worldTx, x2, y2, z2);
			FrameHelper::SetTranslation(worldTx, lk);
			robot->bodies[index[1]]->SetWorldTx(worldTx);

			if (IsDumpEnabled(left))
			{
				WindowsUtility::Debug(L"X2\t%+.10f\t%+.10f\t%+.10f\n", x1.x, x1.y, x1.z);
				WindowsUtility::Debug(L"Y2\t%+.10f\t%+.10f\t%+.10f\n", y1.x, y1.y, y1.z);
				WindowsUtility::Debug(L"Z2\t%+.10f\t%+.10f\t%+.10f\n", z1.x, z1.y, z1.z);
			}
		}
	}

	{
		Matrix4D worldTx = Matrix4D::Identity();
		FrameHelper::Set(worldTx, y, (Vector3D)-x, z);
		FrameHelper::SetTranslation(worldTx, pos);
		robot->bodies[index[2]]->SetWorldTx(worldTx);
	}

	if (IsDumpEnabled(left))
	{
		robot->UpdateWorldTransform();

		auto c = robot->GetLocalRotation("Body");
		auto l1 = robot->GetLocalRotation(left ? "LLeg1" : "RLeg1");
		auto l2 = robot->GetLocalRotation(left ? "LLeg2" : "RLeg2");
		auto f = robot->GetLocalRotation(left ? "LFoot" : "RFoot");

		WindowsUtility::Debug(L"C\t%+.10f\t%+.10f\t%+.10f\n", c.x, c.y, c.z);
		WindowsUtility::Debug(L"L1\t%+.10f\t%+.10f\t%+.10f\n", l1.x, l1.y, l1.z);
		WindowsUtility::Debug(L"L2\t%+.10f\t%+.10f\t%+.10f\n", l2.x, l2.y, l2.z);
		WindowsUtility::Debug(L"F\t%+.10f\t%+.10f\t%+.10f\n", f.x, f.y, f.z);
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

//--------------------------------------------------------------------------
void RobotIK::EnableIKDump(bool enable, bool left)
{
	dump[left ? 0 : 1] = enable;
}