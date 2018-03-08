#pragma once

#include "Vector.h"

struct SolutionCoordinate
{
	// ���⸦ �ε��̿� �� ģȭ�� ���·� �����ؾ� �Ѵ�
	std::pair<Core::Vector3D, Core::Vector3D> body;
	std::pair<Core::Vector3D, Core::Vector3D> foot[2];

	double& At(int i);

	const double& At(int i) const;

	static int VariableCount();

	void Dump() const;
};
