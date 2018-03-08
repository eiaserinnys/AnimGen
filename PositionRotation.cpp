#include "pch.h"
#include "PositionRotation.h"

using namespace std;
using namespace Core;

//------------------------------------------------------------------------------
PositionRotation PositionRotation::Zero()
{ return PositionRotation{ Vector3D(0, 0, 0), (0, 0, 0), }; }

//------------------------------------------------------------------------------
double& PositionRotation::At(int i)
{
	if (0 <= i && i < 3)
	{
		return position.m[i];
	}
	else if (i < 6)
	{
		return rotation.m[i - 3];
	}
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
const double& PositionRotation::At(int i) const
{ return const_cast<PositionRotation*>(this)->At(i); }
