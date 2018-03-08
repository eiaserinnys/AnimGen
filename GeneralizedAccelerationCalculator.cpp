#include "pch.h"
#include "GeneralizedAccelerationCalculator.h"

#include <WindowsUtility.h>
#include <Utility.h>

using namespace std;
using namespace Core;

static const int g_granulity = 1;

//------------------------------------------------------------------------------
GeneralizedAccelerationCalculator::GeneralizedAccelerationCalculator(
	const ISolutionVector* sv,
	int phaseIndexAt,
	bool dump)
{
	vector<pair<double, GeneralizedCoordinate>> gc;

	double timeToQuery = BuildData(gc, sv, phaseIndexAt);

	BuildSpline(gc);

	CalculateAcceleration(timeToQuery);

	if (dump)
	{
		// ดýวม
		WindowsUtility::Debug(L"Source\n");
		for (int i = 0; i < gc.size(); ++i)
		{
			WindowsUtility::Debug(L"%.3f\t", gc[i].first);
			gc[i].second.Dump();
		}

		WindowsUtility::Debug(L"Result\n");
		{
			GeneralizedCoordinate g;

			g.body = spline[0].curve->At(timeToQuery);

			PositionRotation p[] =
			{
				spline[1].curve->At(timeToQuery),
				spline[2].curve->At(timeToQuery),
				spline[3].curve->At(timeToQuery),
				spline[4].curve->At(timeToQuery),
				spline[5].curve->At(timeToQuery),
				spline[6].curve->At(timeToQuery),
			};

			g.leg[0].rot1 = p[0].rotation;
			g.leg[0].len1 = p[0].position.x;
			g.leg[0].rot2 = p[1].rotation;
			g.leg[0].len2 = p[1].position.x;
			g.leg[0].footRot = p[2].rotation;

			g.leg[1].rot1 = p[3].rotation;
			g.leg[1].len1 = p[3].position.x;
			g.leg[1].rot2 = p[4].rotation;
			g.leg[1].len2 = p[4].position.x;
			g.leg[1].footRot = p[5].rotation;

			WindowsUtility::Debug(L"%.3f\t", timeToQuery);
			g.Dump();
		}
	}
}

//------------------------------------------------------------------------------
double GeneralizedAccelerationCalculator::BuildData(
	vector<pair<double, GeneralizedCoordinate>>& gc,
	const ISolutionVector* sv,
	int phaseIndexAt)
{
	int from = Utility::ClampLessThan(phaseIndexAt - 2, 0);
	int to = Utility::ClampGreaterThanOrEqualTo(phaseIndexAt + 2, sv->GetPhaseCount() - 1);

	double timeOffset = sv->GetPhaseTime(from);

	int granulity = g_granulity;

	for (int i = from; i < to; ++i)
	{
		double c = sv->GetPhaseTime(i);

		if (i + 1 < sv->GetPhaseCount())
		{
			double n = sv->GetPhaseTime(i + 1);

			for (int j = 0; j < granulity; ++j)
			{
				double f = j / double(granulity);
				double l = c * (1 - f) + n * f;

				gc.push_back(make_pair(
					l - timeOffset, 
					sv->GeneralizedCoordinateAt(l)));
			}
		}
	}

	gc.push_back(make_pair(
		sv->GetPhaseTime(to) - timeOffset,
		sv->GeneralizedCoordinateAt(sv->GetPhaseTime(to))));

	return sv->GetPhaseTime(phaseIndexAt) - timeOffset;
}

//------------------------------------------------------------------------------
void GeneralizedAccelerationCalculator::BuildSpline(
	const vector<pair<double, GeneralizedCoordinate>>& gc)
{
	for (size_t i = 0; i < gc.size(); ++i)
	{
		spline[0].Append(gc[i].first, gc[i].second.body);
		spline[1].Append(gc[i].first, PositionRotation{ Vector3D(gc[i].second.leg[0].len1, 0, 0), gc[i].second.leg[0].rot1 });
		spline[2].Append(gc[i].first, PositionRotation{ Vector3D(gc[i].second.leg[0].len2, 0, 0), gc[i].second.leg[0].rot2 });
		spline[3].Append(gc[i].first, PositionRotation{ Vector3D(0, 0, 0), gc[i].second.leg[0].footRot });
		spline[4].Append(gc[i].first, PositionRotation{ Vector3D(gc[i].second.leg[1].len1, 0, 0), gc[i].second.leg[1].rot1 });
		spline[5].Append(gc[i].first, PositionRotation{ Vector3D(gc[i].second.leg[1].len2, 0, 0), gc[i].second.leg[1].rot2 });
		spline[6].Append(gc[i].first, PositionRotation{ Vector3D(0, 0, 0), gc[i].second.leg[1].footRot });
	}

	for (int i = 0; i < COUNT_OF(spline); ++i)
	{
		spline[i].SetTimestep(0.5 / g_granulity);
		spline[i].Update();
	}
}

//------------------------------------------------------------------------------
void GeneralizedAccelerationCalculator::CalculateAcceleration(double timeToQuery)
{
	acc.body = spline[0].curve->AccelerationAt(timeToQuery);

	PositionRotation p[] =
	{
		spline[1].curve->AccelerationAt(timeToQuery),
		spline[2].curve->AccelerationAt(timeToQuery),
		spline[3].curve->AccelerationAt(timeToQuery),
		spline[4].curve->AccelerationAt(timeToQuery),
		spline[5].curve->AccelerationAt(timeToQuery),
		spline[6].curve->AccelerationAt(timeToQuery),
	};

	acc.leg[0].rot1 = p[0].rotation;
	acc.leg[0].len1 = p[0].position.x;
	acc.leg[0].rot2 = p[1].rotation;
	acc.leg[0].len2 = p[1].position.x;
	acc.leg[0].footRot = p[2].rotation;

	acc.leg[1].rot1 = p[3].rotation;
	acc.leg[1].len1 = p[3].position.x;
	acc.leg[1].rot2 = p[4].rotation;
	acc.leg[1].len2 = p[4].position.x;
	acc.leg[1].footRot = p[5].rotation;
}

//------------------------------------------------------------------------------
GeneralizedCoordinate GeneralizedAccelerationCalculator::Get()
{
	return acc;
}
