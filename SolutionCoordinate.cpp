#include "pch.h"
#include "SolutionCoordinate.h"

//------------------------------------------------------------------------------
double& SolutionCoordinate::At(int i)
{
	if (0 <= i && i < VariableCount())
	{
		if (i < 3) { return body.first.m[i]; }
		if (i < 6) { return body.second.m[i - 3]; }
		if (i < 9) { return foot[0].first.m[i - 6]; }
		if (i < 12) { return foot[0].second.m[i - 9]; }
		if (i < 15) { return foot[1].first.m[i - 12]; }
		if (i < 18) { return foot[1].second.m[i - 15]; }
	}
	throw std::invalid_argument("out of range");
}

//------------------------------------------------------------------------------
const double& SolutionCoordinate::At(int i) const
{ return const_cast<SolutionCoordinate*>(this)->At(i); }

//------------------------------------------------------------------------------
int SolutionCoordinate::VariableCount()
{
	return
		// body
		3 + 3 +
		// foot
		(3 + 3) * 2;
}
