#pragma once

#include "Vector.h"

struct GeneralCoordinate
{
	struct Leg
	{
		Core::Vector3D rot1;
		double len1;

		Core::Vector3D rot2;
		double len2;

		Core::Vector3D footRot;

		void FillZero();

		double SquaredLength() const;
	};

	std::pair<Core::Vector3D, Core::Vector3D> body;

	Leg leg[2];

	double SquaredLength() const;

	void MakeNear(const GeneralCoordinate& pivot);

	void Clear();

	void Dump() const;

	void Dump_() const;
};

GeneralCoordinate::Leg operator + (const GeneralCoordinate::Leg& lhs, const GeneralCoordinate::Leg& rhs);
GeneralCoordinate::Leg operator - (const GeneralCoordinate::Leg& lhs, const GeneralCoordinate::Leg& rhs);
GeneralCoordinate::Leg operator * (double lhs, const GeneralCoordinate::Leg& rhs);
GeneralCoordinate::Leg operator * (const GeneralCoordinate::Leg& lhs, double rhs);
GeneralCoordinate::Leg operator * (const GeneralCoordinate::Leg& lhs, const GeneralCoordinate::Leg& rhs);
GeneralCoordinate::Leg operator / (const GeneralCoordinate::Leg& lhs, double rhs);
bool operator == (const GeneralCoordinate::Leg& lhs, const GeneralCoordinate::Leg& rhs);

GeneralCoordinate operator + (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs);
GeneralCoordinate operator - (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs);
GeneralCoordinate operator * (double lhs, const GeneralCoordinate& rhs);
GeneralCoordinate operator * (const GeneralCoordinate& lhs, double rhs);
GeneralCoordinate operator * (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs);
GeneralCoordinate operator / (const GeneralCoordinate& lhs, double rhs);

bool operator == (const GeneralCoordinate& lhs, const GeneralCoordinate& rhs);
