#include "pch.h"
#include "SolutionVector.h"

using namespace std;
using namespace Core;

SolutionVector SolutionVector::BuildTest(const SolutionCoordinate& init)
{
	SolutionVector v;

	auto delta = Vector3D(5, 0, 0);

	int phases = 20;

	for (int i = 0; i < phases; ++i)
	{
		double factor = (double)i / (phases - 1);

		SolutionCoordinate nc = init;

		auto move = Lerp(Vector3D(0, 0, 0), delta, factor).Evaluate();

		nc.bodyPos += move;
		nc.footPos[0] += move;
		nc.footPos[1] += move;

		v.coords.push_back(nc);
	}

	return v;
}
