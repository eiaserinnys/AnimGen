#include "pch.h"
#include "SolutionVector.h"

using namespace std;
using namespace Core;

SolutionVector SolutionVector::BuildTest(const SolutionCoordinate& init)
{
	SolutionVector v;

	auto delta = Vector3D(2, 0, 0);
	auto deltaF = Vector3D(1.5, 0.5f, 0);
	auto deltaF2 = Vector3D(3, 0, 0);

	int phases = 8;

	for (int i = 0; i <= phases; ++i)
	{
		double factor = (double)i / phases;

		SolutionCoordinate nc = init;

		auto move = Lerp(Vector3D(0, 0, 0), delta, factor).Evaluate();
		auto moveF = Lerp(Vector3D(0, 0, 0), deltaF, factor).Evaluate();
		auto moveF2 = Lerp(Vector3D(0, 0.5, 0), deltaF2, factor).Evaluate();

		nc.bodyPos += move;
		nc.footPos[0] += moveF;
		nc.footPos[1] += moveF2;

		v.coords.push_back(make_pair(i * 0.5, nc));
	}

	return v;
}
