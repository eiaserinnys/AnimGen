#include "pch.h"
#include "ErrorFunction.h"

using namespace std;
using namespace Core;
using namespace Eigen;

//------------------------------------------------------------------------------
void ErrorFunction::StartUp(int funcCount_)
{
	funcCount = funcCount_;

	matF.reset(new Matrix<double, Dynamic, 1>(funcCount));
}

//------------------------------------------------------------------------------
void ErrorFunction::CleanUp()
{
	//log = nullptr;
	matF.reset(nullptr);
}

//------------------------------------------------------------------------------
void ErrorFunction::Begin()
{
	pY = 0;
}

//------------------------------------------------------------------------------
void ErrorFunction::End()
{
	assert(pY == funcCount);
}

//------------------------------------------------------------------------------
double ErrorFunction::Set(double v, const Coefficient& w)
{
	assert(isfinite(v));

	(*matF)(pY++) = v * w.Sqrt();
	return v * v * (double)w;
}
