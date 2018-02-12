#pragma once

#include "Vector.h"
#include "Matrix.h"

class Robot;

struct GeneralCoordinate
{
	struct Leg
	{
		Core::Vector3D rot1;
		double len1;

		Core::Vector3D rot2;
		double len2;

		Core::Vector3D footRot;
	};

	Core::Vector3D bodyPos;
	Core::Vector3D bodyRot;

	Leg leg[2];
};

struct SolutionCoordinate
{
	Core::Vector3D bodyPos;
	Core::Vector3D bodyRot;

	Core::Vector3D footPos[2];
	Core::Vector3D footRot[2];

	void Dump() const;
};

struct RobotCoordinate
{
	GeneralCoordinate ToGeneralCoordinate(Robot* robot) const;
	void SetTransform(Robot* robot, const GeneralCoordinate& coord, bool validate) const;

	SolutionCoordinate ToSolutionCoordinate(Robot* robot) const;
	void SetTransform(Robot* robot, const SolutionCoordinate& solution, bool validate) const;
};