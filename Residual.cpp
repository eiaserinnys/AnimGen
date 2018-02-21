#include "pch.h"
#include "Residual.h"

using namespace std;
using namespace Core;
using namespace Eigen;

//------------------------------------------------------------------------------
void Residual::StartUp(int funcCount_)
{
	funcCount = funcCount_;

	matF.reset(new Matrix<double, Dynamic, 1>(funcCount));
}

//------------------------------------------------------------------------------
void Residual::CleanUp()
{
	//log = nullptr;
	matF.reset(nullptr);
}

//------------------------------------------------------------------------------
void Residual::Begin()
{
	pY = 0;
}

//------------------------------------------------------------------------------
void Residual::End()
{
	assert(pY == funcCount);
}

//------------------------------------------------------------------------------
double Residual::Set(double v, const Coefficient& w)
{
	assert(isfinite(v));

	(*matF)(pY++) = v * w.Sqrt();
	return v * v * (double)w;
}
