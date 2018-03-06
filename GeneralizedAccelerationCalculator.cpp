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
	vector<pair<double, GeneralCoordinate>> gc;

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
			GeneralCoordinate g;

			g.body = spline[0].curve->At(timeToQuery);

			pair<Vector3D, Vector3D> p[] =
			{
				spline[1].curve->At(timeToQuery),
				spline[2].curve->At(timeToQuery),
				spline[3].curve->At(timeToQuery),
				spline[4].curve->At(timeToQuery),
				spline[5].curve->At(timeToQuery),
				spline[6].curve->At(timeToQuery),
			};

			g.leg[0].rot1 = p[0].second;
			g.leg[0].len1 = p[0].first.x;
			g.leg[0].rot2 = p[1].second;
			g.leg[0].len2 = p[1].first.x;
			g.leg[0].footRot = p[2].second;

			g.leg[1].rot1 = p[3].second;
			g.leg[1].len1 = p[3].first.x;
			g.leg[1].rot2 = p[4].second;
			g.leg[1].len2 = p[4].first.x;
			g.leg[1].footRot = p[5].second;

			WindowsUtility::Debug(L"%.3f\t", timeToQuery);
			g.Dump();
		}
	}
}

//------------------------------------------------------------------------------
double GeneralizedAccelerationCalculator::BuildData(
	vector<pair<double, GeneralCoordinate>>& gc,
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
					sv->GeneralCoordinateAt(l)));
			}
		}
	}

	gc.push_back(make_pair(
		sv->GetPhaseTime(to) - timeOffset,
		sv->GeneralCoordinateAt(sv->GetPhaseTime(to))));

	return sv->GetPhaseTime(phaseIndexAt) - timeOffset;
}

//------------------------------------------------------------------------------
void GeneralizedAccelerationCalculator::BuildSpline(
	const vector<pair<double, GeneralCoordinate>>& gc)
{
	for (size_t i = 0; i < gc.size(); ++i)
	{
		spline[0].Append(gc[i].first, gc[i].second.body);
		spline[1].Append(gc[i].first, make_pair(Vector3D(gc[i].second.leg[0].len1, 0, 0), gc[i].second.leg[0].rot1));
		spline[2].Append(gc[i].first, make_pair(Vector3D(gc[i].second.leg[0].len2, 0, 0), gc[i].second.leg[0].rot2));
		spline[3].Append(gc[i].first, make_pair(Vector3D(0, 0, 0), gc[i].second.leg[0].footRot));
		spline[4].Append(gc[i].first, make_pair(Vector3D(gc[i].second.leg[1].len1, 0, 0), gc[i].second.leg[1].rot1));
		spline[5].Append(gc[i].first, make_pair(Vector3D(gc[i].second.leg[1].len2, 0, 0), gc[i].second.leg[1].rot2));
		spline[6].Append(gc[i].first, make_pair(Vector3D(0, 0, 0), gc[i].second.leg[1].footRot));
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

	pair<Vector3D, Vector3D> p[] =
	{
		spline[1].curve->AccelerationAt(timeToQuery),
		spline[2].curve->AccelerationAt(timeToQuery),
		spline[3].curve->AccelerationAt(timeToQuery),
		spline[4].curve->AccelerationAt(timeToQuery),
		spline[5].curve->AccelerationAt(timeToQuery),
		spline[6].curve->AccelerationAt(timeToQuery),
	};

	acc.leg[0].rot1 = p[0].second;
	acc.leg[0].len1 = p[0].first.x;
	acc.leg[0].rot2 = p[1].second;
	acc.leg[0].len2 = p[1].first.x;
	acc.leg[0].footRot = p[2].second;

	acc.leg[1].rot1 = p[3].second;
	acc.leg[1].len1 = p[3].first.x;
	acc.leg[1].rot2 = p[4].second;
	acc.leg[1].len2 = p[4].first.x;
	acc.leg[1].footRot = p[5].second;
}

//------------------------------------------------------------------------------
GeneralCoordinate GeneralizedAccelerationCalculator::Get()
{
	return acc;
}
