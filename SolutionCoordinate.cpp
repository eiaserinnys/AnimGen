#include "pch.h"
#include "SolutionCoordinate.h"

#include <Utility.h>

//------------------------------------------------------------------------------
SolutionCoordinate::SolutionCoordinate()
{
}

//------------------------------------------------------------------------------
SolutionCoordinate::SolutionCoordinate(const SolutionCoordinate& rhs)
{
	operator = (rhs);
}

//------------------------------------------------------------------------------
SolutionCoordinate& SolutionCoordinate::operator = (const SolutionCoordinate& rhs)
{
	for (int i = 0; i < COUNT_OF(joint); ++i)
	{
		joint[i] = rhs.joint[i];
	}

	return *this;
}

//------------------------------------------------------------------------------
double& SolutionCoordinate::At(int i)
{
	if (0 <= i && i < VariableCount())
	{
		int ind = i / 6;
		int ofs = i % 6;

		return ofs < 3 ? 
			joint[ind].position.m[ofs] : 
			joint[ind].rotation.m[ofs - 3];
	}
	throw std::invalid_argument("out of range");
}

//------------------------------------------------------------------------------
const double& SolutionCoordinate::At(int i) const
{ return const_cast<SolutionCoordinate*>(this)->At(i); }

//------------------------------------------------------------------------------
int SolutionCoordinate::VariableCount()
{
	return 3 * 2 * COUNT_OF(joint);
}
