#pragma once

#include <vector>

#include "RobotCoordinate.h"
#include "HermiteSpline.h"
#include "ClampedSpline.h"
#include "ClampedSplineFixedStep.h"

class IRobot;

struct SolutionVector
{
	SolutionVector();

	SolutionCoordinate At(double t);

	static double Timestep();

	void Update();

	void UpdateSpline();

	void UpdateGenericCoordinates();

	void Rearrange(
		GeneralCoordinate& prev,
		GeneralCoordinate& current,
		GeneralCoordinate& next);

	static SolutionVector* BuildTest(const SolutionCoordinate& init);

	// 솔루션 벡터
	std::vector<std::pair<double, SolutionCoordinate>> coords;

	struct Spline
	{
		std::unique_ptr<ISpline> curve;
		std::vector<double> time;
		std::vector<Core::Vector3D> pos;
		std::vector<Core::Vector3D> rot;

		void Update()
		{
			//curve.reset(IHermiteSpline::Create(pos, rot));
			//curve.reset(IClampedSpline::Create(time, pos, rot));
			curve.reset(IClampedSplineFixedStep::Create(Timestep(), pos, rot));
		}

		void Append(double t, const std::pair<Core::Vector3D, Core::Vector3D>& p)
		{
			time.push_back(t);
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