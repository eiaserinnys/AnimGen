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

		void FillZero()
		{
			rot1.FillZero();
			len1 = 0;

			rot2.FillZero();
			len2 = 0;

			footRot.FillZero();
		}
	};

	std::pair<Core::Vector3D, Core::Vector3D> body;

	Leg leg[2];

	void MakeNear(const GeneralCoordinate& pivot);

	void Clear();

	void Dump() const;
};

GeneralCoordinate::Leg operator + (const GeneralCoordinate::Leg& lhs, const GeneralCoordinate::Leg& rhs);
GeneralCoordinate::Leg operator - (const GeneralCoordinate::Leg& lhs, const GeneralCoordinate::Leg& rhs);
GeneralCoordinate::Leg operator * (const GeneralCoordinate::Leg& lhs, double rhs);
GeneralCoordinate::Leg operator / (const GeneralCoordinate::Leg& lhs, double rhs);

GeneralCoordinate operator + (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs);
GeneralCoordinate operator - (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs);
GeneralCoordinate operator * (const GeneralCoordinate& lhs, double rhs);
GeneralCoordinate operator / (const GeneralCoordinate& lhs, double rhs);

struct SolutionCoordinate
{
	std::pair<Core::Vector3D, Core::Vector3D> body;
	std::pair<Core::Vector3D, Core::Vector3D> foot[2];

	void Dump() const;
};

struct RobotCoordinate
{
	GeneralCoordinate ToGeneralCoordinate(Robot* robot) const;
	bool SetTransform(Robot* robot, const GeneralCoordinate& coord, bool validate) const;

	SolutionCoordinate ToSolutionCoordinate(Robot* robot) const;
	void SetTransform(Robot* robot, const SolutionCoordinate& solution, bool validate) const;
};