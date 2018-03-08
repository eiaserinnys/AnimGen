#pragma once

#include "Vector.h"

struct SolutionCoordinate
{
	// 여기를 인덱싱에 더 친화된 형태로 구현해야 한다
	std::pair<Core::Vector3D, Core::Vector3D> body;
	std::pair<Core::Vector3D, Core::Vector3D> foot[2];

	double& At(int i);

	const double& At(int i) const;

	static int VariableCount();

	void Dump() const;
};
