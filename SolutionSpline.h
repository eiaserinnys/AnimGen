#pragma once

#include <memory>
#include "ClampedSplineFixedStep.h"

//------------------------------------------------------------------------------
struct SolutionSpline
{
	const float g_timeStep = 0.5;

	std::unique_ptr<ISpline> curve;
	std::vector<Core::Vector3D> pos;
	std::vector<Core::Vector3D> rot;

	SolutionSpline() = default;

	SolutionSpline& operator = (const SolutionSpline& rhs)
	{
		pos = rhs.pos;
		rot = rhs.rot;
		Update();
		return *this;
	}

	void SetValue(int index, int channel, double v)
	{
		curve->SetValue(index, channel, v);
	}

	void Update()
	{
		curve.reset(IClampedSplineFixedStep::Create(g_timeStep, pos, rot));
	}

	void Append(double t, const std::pair<Core::Vector3D, Core::Vector3D>& p)
	{
		pos.push_back(p.first);
		rot.push_back(p.second);
	}
};
