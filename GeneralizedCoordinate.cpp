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
void GeneralizedCoordinate::Leg::FillZero()
{
	rot1.FillZero();
	len1 = 0;

	rot2.FillZero();
	len2 = 0;

	footRot.FillZero();
}

//--------------------------------------------------------------------------
double GeneralizedCoordinate::Leg::SquaredLength() const
{
	return
		Core::SquaredLength(rot1) +
		len1 * len1 +
		Core::SquaredLength(rot2) +
		len2 * len2 +
		Core::SquaredLength(footRot);
}

//--------------------------------------------------------------------------
GeneralizedCoordinate::Leg operator + (
	const GeneralizedCoordinate::Leg& lhs,
	const GeneralizedCoordinate::Leg& rhs)
{
	GeneralizedCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 + rhs.rot1;
	ret.len1 = lhs.len1 + rhs.len1;
	ret.rot2 = lhs.rot2 + rhs.rot2;
	ret.len2 = lhs.len2 + rhs.len2;
	ret.footRot = lhs.footRot + rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate::Leg operator - (
	const GeneralizedCoordinate::Leg& lhs,
	const GeneralizedCoordinate::Leg& rhs)
{
	GeneralizedCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 - rhs.rot1;
	ret.len1 = lhs.len1 - rhs.len1;
	ret.rot2 = lhs.rot2 - rhs.rot2;
	ret.len2 = lhs.len2 - rhs.len2;
	ret.footRot = lhs.footRot - rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate::Leg operator * (
	const GeneralizedCoordinate::Leg& lhs,
	double rhs)
{
	GeneralizedCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 * rhs;
	ret.len1 = lhs.len1 * rhs;
	ret.rot2 = lhs.rot2 * rhs;
	ret.len2 = lhs.len2 * rhs;
	ret.footRot = lhs.footRot * rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate::Leg operator * (
	double lhs,
	const GeneralizedCoordinate::Leg& rhs)
{
	GeneralizedCoordinate::Leg ret;

	ret.rot1 = lhs * rhs.rot1;
	ret.len1 = lhs * rhs.len1;
	ret.rot2 = lhs * rhs.rot2;
	ret.len2 = lhs * rhs.len2;
	ret.footRot = lhs * rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate::Leg operator * (
	const GeneralizedCoordinate::Leg& lhs,
	const GeneralizedCoordinate::Leg& rhs)
{
	GeneralizedCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 * rhs.rot1;
	ret.len1 = lhs.len1 * rhs.len1;
	ret.rot2 = lhs.rot2 * rhs.rot2;
	ret.len2 = lhs.len2 * rhs.len2;
	ret.footRot = lhs.footRot * rhs.footRot;

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate::Leg operator / (
	const GeneralizedCoordinate::Leg& lhs,
	double rhs)
{
	GeneralizedCoordinate::Leg ret;

	ret.rot1 = lhs.rot1 / rhs;
	ret.len1 = lhs.len1 / rhs;
	ret.rot2 = lhs.rot2 / rhs;
	ret.len2 = lhs.len2 / rhs;
	ret.footRot = lhs.footRot / rhs;

	return ret;
}

//--------------------------------------------------------------------------
bool operator == (
	const GeneralizedCoordinate::Leg& lhs,
	const GeneralizedCoordinate::Leg& rhs)
{
	return
		lhs.rot1 == rhs.rot1 &&
		lhs.len1 == rhs.len1 &&
		lhs.rot2 == rhs.rot2 &&
		lhs.len2 == rhs.len2 &&
		lhs.footRot == rhs.footRot;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate operator - (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs)
{
	GeneralizedCoordinate ret;

	ret.body.position = lhs.body.position - rhs.body.position;
	ret.body.rotation = lhs.body.rotation - rhs.body.rotation;
	ret.leg[0] = lhs.leg[0] - rhs.leg[0];
	ret.leg[1] = lhs.leg[1] - rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate operator + (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs)
{
	GeneralizedCoordinate ret;

	ret.body.position = lhs.body.position + rhs.body.position;
	ret.body.rotation = lhs.body.rotation + rhs.body.rotation;
	ret.leg[0] = lhs.leg[0] + rhs.leg[0];
	ret.leg[1] = lhs.leg[1] + rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate operator / (const GeneralizedCoordinate& lhs, double rhs)
{
	GeneralizedCoordinate ret;

	ret.body.position = lhs.body.position / rhs;
	ret.body.rotation = lhs.body.rotation / rhs;
	ret.leg[0] = lhs.leg[0] / rhs;
	ret.leg[1] = lhs.leg[1] / rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate operator * (const GeneralizedCoordinate& lhs, double rhs)
{
	GeneralizedCoordinate ret;

	ret.body.position = lhs.body.position * rhs;
	ret.body.rotation = lhs.body.rotation * rhs;
	ret.leg[0] = lhs.leg[0] * rhs;
	ret.leg[1] = lhs.leg[1] * rhs;

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate operator * (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs)
{
	GeneralizedCoordinate ret;

	ret.body.position = lhs.body.position * rhs.body.position;
	ret.body.rotation = lhs.body.rotation * rhs.body.rotation;
	ret.leg[0] = lhs.leg[0] * rhs.leg[0];
	ret.leg[1] = lhs.leg[1] * rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
GeneralizedCoordinate operator * (double lhs, const GeneralizedCoordinate& rhs)
{
	GeneralizedCoordinate ret;

	ret.body.position = lhs * rhs.body.position;
	ret.body.rotation = lhs * rhs.body.rotation;
	ret.leg[0] = lhs * rhs.leg[0];
	ret.leg[1] = lhs * rhs.leg[1];

	return ret;
}

//--------------------------------------------------------------------------
bool operator == (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs)
{
	return
		lhs.body.position == rhs.body.position &&
		lhs.body.rotation == rhs.body.rotation &&
		lhs.leg[0] == rhs.leg[0] &&
		lhs.leg[1] == rhs.leg[1];
}

//------------------------------------------------------------------------------
void GeneralizedCoordinate::MakeNear(const GeneralizedCoordinate& pivot)
{
	ExponentialMap::MakeNearRotation(pivot.body.rotation, body.rotation);

	for (int i = 0; i < COUNT_OF(leg); ++i)
	{
		ExponentialMap::MakeNearRotation(pivot.leg[i].rot1, leg[i].rot1);
		ExponentialMap::MakeNearRotation(pivot.leg[i].rot2, leg[i].rot2);
		ExponentialMap::MakeNearRotation(pivot.leg[i].footRot, leg[i].footRot);
	}
}

//--------------------------------------------------------------------------
void GeneralizedCoordinate::Clear()
{
	body.position.FillZero();
	body.rotation.FillZero();

	leg[0].FillZero();
	leg[1].FillZero();
}

//--------------------------------------------------------------------------
void GeneralizedCoordinate::Dump() const
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
		body.position.x, body.position.y, body.position.z,
		body.rotation.x, body.rotation.y, body.rotation.z,
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
			body.position.x, body.position.y, body.position.y,
			body.rotation.x, body.rotation.y, body.rotation.y);
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
void GeneralizedCoordinate::Dump_() const
{
	{
		WindowsUtility::Debug(
			L"%e\t%e\t%e\t%e\t",
			leg[1].rot2.x, leg[1].rot2.y, leg[1].rot2.z, leg[1].len2);
		WindowsUtility::Debug(L"\n");
	}
}

//--------------------------------------------------------------------------
double GeneralizedCoordinate::SquaredLength() const
{
	return
		Core::SquaredLength(body.position) +
		Core::SquaredLength(body.rotation)
		+
		leg[0].SquaredLength() +
		leg[1].SquaredLength();
}
