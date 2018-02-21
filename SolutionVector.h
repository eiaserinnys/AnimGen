#pragma once

#include <vector>

#include "Robot.h"
#include "RobotCoordinate.h"
#include "HermiteSpline.h"
#include "ClampedSpline.h"
#include "ClampedSplineFixedStep.h"

class IRobot;

struct SolutionVector
{
	SolutionVector();
	SolutionVector(const SolutionVector& rhs);

	int VariableCount() const 
	{ 
		return (int) coords.size() * SolutionCoordinate::VariableCount(); 
	}

	double GetVariableAt(int i) const
	{
		int varPerEntry = SolutionCoordinate::VariableCount();

		if (0 <= i && i < varPerEntry * coords.size())
		{
			int ei = i / varPerEntry;
			int ej = i % varPerEntry;
			return coords[ei].second.At(ej);
		}
		else
		{
			throw std::invalid_argument("");
		}
	}

	void SetVariableAt(int i, double v)
	{
		int varPerEntry = SolutionCoordinate::VariableCount();

		if (0 <= i && i < varPerEntry * coords.size())
		{
			int ei = i / varPerEntry;
			int ej = i % varPerEntry;
			coords[ei].second.At(ej) = v;

			// 여기서 스플라인 업데이트를 해줘야 한다
		}
		else
		{
			throw std::invalid_argument("");
		}
	}

	SolutionCoordinate At(double t) const;

	GeneralCoordinate GeneralAccAt(double t) const;

	static double Timestep();

	void Update();

	void UpdateSpline();

	void Dump();

	static SolutionVector* BuildTest(const SolutionCoordinate& init);

public:
	struct Spline
	{
		std::unique_ptr<ISpline> curve;
		std::vector<Core::Vector3D> pos;
		std::vector<Core::Vector3D> rot;

		Spline() = default;
		
		Spline& operator = (const Spline& rhs)
		{
			pos = rhs.pos;
			rot = rhs.rot;
			Update();
			return *this;
		}

		void Update()
		{
			curve.reset(IClampedSplineFixedStep::Create(Timestep(), pos, rot));
		}

		void Append(double t, const std::pair<Core::Vector3D, Core::Vector3D>& p)
		{
			pos.push_back(p.first);
			rot.push_back(p.second);
		}
	};

	struct Splines
	{
		Spline body;
		Spline foot[2];

		Splines& operator = (const Splines& rhs)
		{
			body = rhs.body;
			foot[0] = rhs.foot[0];
			foot[1] = rhs.foot[1];
			return *this;
		}
	};

	// 솔루션 벡터
	std::vector<std::pair<double, SolutionCoordinate>> coords;

	// 솔루션 벡터 -> 스플라인
	Splines splines;

private:
	// 내부용 로봇
	std::unique_ptr<IRobot> robot;
};