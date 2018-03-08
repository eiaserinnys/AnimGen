#pragma once

#include "PositionRotation.h"

struct SolutionCoordinate
{
	SolutionCoordinate();

	SolutionCoordinate(const SolutionCoordinate& rhs);

	SolutionCoordinate& operator = (const SolutionCoordinate& rhs);

	struct Joint
	{
		enum Value
		{
			Body,
			LeftFoot,
			RightFoot,

			Count, 
		};
	};

	// ���⸦ �ε��̿� �� ģȭ�� ���·� �����ؾ� �Ѵ�
	union
	{
		struct
		{
			PositionRotation body;
			PositionRotation foot[2];
		};

		PositionRotation joint[3];
	};

	double& At(int i);

	const double& At(int i) const;

	static int VariableCount();

	void Dump() const;
};
