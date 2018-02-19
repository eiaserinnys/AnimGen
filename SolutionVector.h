#pragma once

#include <vector>

#include "RobotCoordinate.h"
#include "HermiteSpline.h"

class IRobot;

struct SolutionVector
{
	SolutionVector();

	SolutionCoordinate AtByFactor(double f);
	SolutionCoordinate AtByTime(double t);

	double Timestep();

	void Update();

	void UpdateSpline();

	void UpdateGenericCoordinates();

	static SolutionVector* BuildTest(const SolutionCoordinate& init);

	// 솔루션 벡터
	std::vector<std::pair<double, SolutionCoordinate>> coords;

	struct Spline
	{
		std::unique_ptr<IHermiteSpline> curve;
		std::vector<Core::Vector3D> pos;
		std::vector<Core::Vector3D> rot;

		void Update()
		{
			curve.reset(IHermiteSpline::Create(pos, rot));
		}

		void Append(const std::pair<Core::Vector3D, Core::Vector3D>& p)
		{
			pos.push_back(p.first);
			rot.push_back(p.second);
		}
	};

	struct Splines
	{
		Spline body;
		Spline foot[2];
	};

	// 솔루션 벡터 -> 스플라인
	Splines splines;

	// 스플라인 -> 일반화 좌표
	std::vector<GeneralCoordinate> gAcc;

private:
	// 내부용 로봇
	std::unique_ptr<IRobot> robot;
};