#pragma once

#include <vector>

#include "RobotCoordinate.h"
#include "HermiteSpline.h"

class IRobot;

struct SolutionVector
{
	void UpdateSpline();

	void CalculateGenericCoordinates(IRobot* robot);

	static SolutionVector BuildTest(const SolutionCoordinate& init);

	std::vector<std::pair<double, SolutionCoordinate>> coords;

	struct Spline
	{
		std::unique_ptr<IHermiteSpline> curve;
		std::vector<Core::Vector3D> pos;
		std::vector<Core::Vector3D> rot;

		std::unique_ptr<std::function<void()>> callback;

		void Update()
		{
			curve.reset(IHermiteSpline::Create(pos, rot));

			if (callback.get() != nullptr)
			{
				(*callback)();
			}
		}
	};

	struct Splines
	{
		Spline body;
		Spline foot[2];
	};

	Splines splines;
};