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

		double SquaredLength() const
		{
			return
				Core::SquaredLength(rot1) +
				len1 * len1 +
				Core::SquaredLength(rot2) +
				len2 * len2 +
				Core::SquaredLength(footRot);
		}
	};

	std::pair<Core::Vector3D, Core::Vector3D> body;

	Leg leg[2];

	double SquaredLength() const;

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
	// 여기를 인덱싱에 더 친화된 형태로 구현해야 한다
	std::pair<Core::Vector3D, Core::Vector3D> body;
	std::pair<Core::Vector3D, Core::Vector3D> foot[2];

	double& At(int i)
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

	const double& At(int i) const
	{
		return const_cast<SolutionCoordinate*>(this)->At(i);
	}

	static int VariableCount() 
	{ 
		return
			// body
			3 + 3 +
			// foot
			(3 + 3) * 2;
	}

	void Dump() const;
};

struct RobotCoordinate
{
	GeneralCoordinate ToGeneralCoordinate(Robot* robot) const;
	bool SetTransform(Robot* robot, const GeneralCoordinate& coord, bool validate) const;

	SolutionCoordinate ToSolutionCoordinate(Robot* robot) const;
	void SetTransform(Robot* robot, const SolutionCoordinate& solution, bool validate) const;
};