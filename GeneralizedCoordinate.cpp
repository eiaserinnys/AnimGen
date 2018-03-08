#include "pch.h"
#include "GeneralizedCoordinate.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "ExponentialMap.h"

using namespace std;
using namespace Core;

//--------------------------------------------------------------------------
static bool operator == (const Vector3D& lhs, const Vector3D& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y &&lhs.z == rhs.z;
}

//--------------------------------------------------------------------------
void GeneralCoordinate::Leg::FillZero()
{
	rot1.FillZero();
	len1 = 0;

	rot2.FillZero();
	len2 = 0;

	footRot.FillZero();
}

//--------------------------------------------------------------------------
double GeneralCoordinate::Leg::SquaredLength() const
{
	return
		Core::SquaredLength(rot1) +
		len1 * len1 +
		Core::SquaredLength(rot2) +
		len2 * len2 +
		Core::SquaredLength(footRot);
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator + (
	const GeneralCoordinate::Leg& lhs,
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 + rhs.rot1;
	ret.len1 = lhs.len1 + rhs.len1;
	ret.rot2 = lhs.rot2 + rhs.rot2;
	ret.len2 = lhs.len2 + rhs.len2;
	ret.footRot = lhs.footRot + rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator - (
	const GeneralCoordinate::Leg& lhs,
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 - rhs.rot1;
	ret.len1 = lhs.len1 - rhs.len1;
	ret.rot2 = lhs.rot2 - rhs.rot2;
	ret.len2 = lhs.len2 - rhs.len2;
	ret.footRot = lhs.footRot - rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator * (
	const GeneralCoordinate::Leg& lhs,
	double rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 * rhs;
	ret.len1 = lhs.len1 * rhs;
	ret.rot2 = lhs.rot2 * rhs;
	ret.len2 = lhs.len2 * rhs;
	ret.footRot = lhs.footRot * rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator * (
	double lhs,
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs * rhs.rot1;
	ret.len1 = lhs * rhs.len1;
	ret.rot2 = lhs * rhs.rot2;
	ret.len2 = lhs * rhs.len2;
	ret.footRot = lhs * rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator * (
	const GeneralCoordinate::Leg& lhs,
	const GeneralCoordinate::Leg& rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 * rhs.rot1;
	ret.len1 = lhs.len1 * rhs.len1;
	ret.rot2 = lhs.rot2 * rhs.rot2;
	ret.len2 = lhs.len2 * rhs.len2;
	ret.footRot = lhs.footRot * rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate::Leg operator / (
	const GeneralCoordinate::Leg& lhs,
	double rhs)
{
	GeneralCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 / rhs;
	ret.len1 = lhs.len1 / rhs;
	ret.rot2 = lhs.rot2 / rhs;
	ret.len2 = lhs.len2 / rhs;
	ret.footRot = lhs.footRot / rhs;

	return ret;
}

//--------------------------------------------------------------------------
bool operator == (
	const GeneralCoordinate::Leg& lhs,
	const GeneralCoordinate::Leg& rhs)
{
	return
		lhs.rot1 == rhs.rot1 &&
		lhs.len1 == rhs.len1 &&
		lhs.rot2 == rhs.rot2 &&
		lhs.len2 == rhs.len2 &&
		lhs.footRot == rhs.footRot;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator - (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first - rhs.body.first;
	ret.body.second = lhs.body.second - rhs.body.second;
	ret.leg[0] = lhs.leg[0] - rhs.leg[0];
	ret.leg[1] = lhs.leg[1] - rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator + (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first + rhs.body.first;
	ret.body.second = lhs.body.second + rhs.body.second;
	ret.leg[0] = lhs.leg[0] + rhs.leg[0];
	ret.leg[1] = lhs.leg[1] + rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator / (const GeneralCoordinate& lhs, double rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first / rhs;
	ret.body.second = lhs.body.second / rhs;
	ret.leg[0] = lhs.leg[0] / rhs;
	ret.leg[1] = lhs.leg[1] / rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator * (const GeneralCoordinate& lhs, double rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first * rhs;
	ret.body.second = lhs.body.second * rhs;
	ret.leg[0] = lhs.leg[0] * rhs;
	ret.leg[1] = lhs.leg[1] * rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator * (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs.body.first * rhs.body.first;
	ret.body.second = lhs.body.second * rhs.body.second;
	ret.leg[0] = lhs.leg[0] * rhs.leg[0];
	ret.leg[1] = lhs.leg[1] * rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralCoordinate operator * (double lhs, const GeneralCoordinate& rhs)
{
	GeneralCoordinate ret;

	ret.body.first = lhs * rhs.body.first;
	ret.body.second = lhs * rhs.body.second;
	ret.leg[0] = lhs * rhs.leg[0];
	ret.leg[1] = lhs * rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
bool operator == (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs)
{
	return
		lhs.body.first == rhs.body.first &&
		lhs.body.second == rhs.body.second &&
		lhs.leg[0] == rhs.leg[0] &&
		lhs.leg[1] == rhs.leg[1];
}

//------------------------------------------------------------------------------
void GeneralCoordinate::MakeNear(const GeneralCoordinate& pivot)
{
	ExponentialMap::MakeNearRotation(pivot.body.second, body.second);

	for (int i = 0; i < COUNT_OF(leg); ++i)
	{
		ExponentialMap::MakeNearRotation(pivot.leg[i].rot1, leg[i].rot1);
		ExponentialMap::MakeNearRotation(pivot.leg[i].rot2, leg[i].rot2);
		ExponentialMap::MakeNearRotation(pivot.leg[i].footRot, leg[i].footRot);
	}
}

//--------------------------------------------------------------------------
void GeneralCoordinate::Clear()
{
	body.first.FillZero();
	body.second.FillZero();

	leg[0].FillZero();
	leg[1].FillZero();
}

//--------------------------------------------------------------------------
void GeneralCoordinate::Dump() const
{
#if 0
	WindowsUtility::Debug(
		L"B(%+.3f,%+.3f,%+.3f) (%+.3f,%+.3f,%+.3f) "
		"LR1(%+.3f,%+.3f,%+.3f) LLen1(%+.3f), "
		"LR2(%+.3f, %+.3f,%+.3f) LLen2(%+.3f), "
		"LFR(%+.3f, %+.3f,%+.3f) "
		"RR1(%+.3f,%+.3f,%+.3f) RLen1(%+.3f) "
		"RR2(%+.3f,%+.3f,%+.3f) RLen2(%+.3f) "
		"RFR(%+.3f,%+.3f,%+.3f)\n",
		body.first.x, body.first.y, body.first.z,
		body.second.x, body.second.y, body.second.z,
		leg[0].rot1.x, leg[0].rot1.y, leg[0].rot1.z, leg[0].len1,
		leg[0].rot2.x, leg[0].rot2.y, leg[0].rot2.z, leg[0].len2,
		leg[0].footRot.x, leg[0].footRot.y, leg[0].footRot.z,
		leg[1].rot1.x, leg[1].rot1.y, leg[1].rot1.z, leg[1].len1,
		leg[1].rot2.x, leg[1].rot2.y, leg[1].rot2.z, leg[1].len2,
		leg[1].footRot.x, leg[1].footRot.y, leg[1].footRot.z);
#else
	{
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t"
			"%e\t%e\t%e\t",
			body.first.x, body.first.y, body.first.y,
			body.second.x, body.second.y, body.second.y);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[0].rot1.x, leg[0].rot1.y, leg[0].rot1.z, leg[0].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[0].rot2.x, leg[0].rot2.y, leg[0].rot2.z, leg[0].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t",
			leg[0].footRot.x, leg[0].footRot.y, leg[0].footRot.z);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[1].rot1.x, leg[1].rot1.y, leg[1].rot1.z, leg[1].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[1].rot2.x, leg[1].rot2.y, leg[1].rot2.z, leg[1].len2);
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t",
			leg[1].footRot.x, leg[1].footRot.y, leg[1].footRot.z);

		WindowsUtility::Debug(L"\n");
	}
#endif
}

//--------------------------------------------------------------------------
void GeneralCoordinate::Dump_() const
{
	{
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[1].rot2.x, leg[1].rot2.y, leg[1].rot2.z, leg[1].len2);
		WindowsUtility::Debug(L"\n");
	}
}

//--------------------------------------------------------------------------
double GeneralCoordinate::SquaredLength() const
{
	return
		Core::SquaredLength(body.first) +
		Core::SquaredLength(body.second)
		+
		leg[0].SquaredLength() +
		leg[1].SquaredLength();
}
