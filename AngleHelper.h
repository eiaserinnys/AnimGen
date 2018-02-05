#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

template <typename FloatType>
struct AngleHelper
{
	static FloatType RadianToDegree(FloatType angle)
	{ return (FloatType)(angle / M_PI * 180); }

	static FloatType DegreeToRadian(FloatType angle)
	{ return (FloatType)(angle / 180 * M_PI); }
};

typedef AngleHelper<float> AngleHelperF;
typedef AngleHelper<double> AngleHelperD;