#pragma once

#include "Vector.h"
#include "PositionRotation.h"

struct GeneralizedCoordinate
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

	PositionRotation body;

	Leg leg[2];

	double SquaredLength() const;

	void MakeNear(const GeneralizedCoordinate& pivot);

	void Clear();

	void Dump() const;

	void Dump_() const;
};

GeneralizedCoordinate::Leg operator + (const GeneralizedCoordinate::Leg& lhs, const GeneralizedCoordinate::Leg& rhs);
GeneralizedCoordinate::Leg operator - (const GeneralizedCoordinate::Leg& lhs, const GeneralizedCoordinate::Leg& rhs);
GeneralizedCoordinate::Leg operator * (double lhs, const GeneralizedCoordinate::Leg& rhs);
GeneralizedCoordinate::Leg operator * (const GeneralizedCoordinate::Leg& lhs, double rhs);
GeneralizedCoordinate::Leg operator * (const GeneralizedCoordinate::Leg& lhs, const GeneralizedCoordinate::Leg& rhs);
GeneralizedCoordinate::Leg operator / (const GeneralizedCoordinate::Leg& lhs, double rhs);
bool operator == (const GeneralizedCoordinate::Leg& lhs, const GeneralizedCoordinate::Leg& rhs);

GeneralizedCoordinate operator + (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs);
GeneralizedCoordinate operator - (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs);
GeneralizedCoordinate operator * (double lhs, const GeneralizedCoordinate& rhs);
GeneralizedCoordinate operator * (const GeneralizedCoordinate& lhs, double rhs);
GeneralizedCoordinate operator * (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs);
GeneralizedCoordinate operator / (const GeneralizedCoordinate& lhs, double rhs);

bool operator == (const GeneralizedCoordinate& lhs, const GeneralizedCoordinate& rhs);
