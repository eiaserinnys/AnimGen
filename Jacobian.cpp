#include "pch.h"
#include "Jacobian.h"

using namespace std;
using namespace Eigen;
using namespace DirectX;

//------------------------------------------------------------------------------
void Jacobian::StartUp(int fn, int var)
{
	matJ.reset(new SparseMatrix<double>(fn, var));
	matJtJ.reset(new SparseMatrix<double>);
	funCount = fn;
	varCount = var;
}

//------------------------------------------------------------------------------
void Jacobian::CleanUp()
{
	matJ.reset(nullptr);
	matJtJ.reset(nullptr);
}

//------------------------------------------------------------------------------
void Jacobian::Begin()
{
	tp.clear();
	pY = 0;
}

//------------------------------------------------------------------------------
void Jacobian::End()
{
	matJ->setFromTriplets(tp.begin(), tp.end());

	assert(pY == funCount);

	*matJtJ = matJ->transpose() * *matJ;
}

//------------------------------------------------------------------------------
void Jacobian::Set(int x, const Coefficient& w, double v)
{
	assert(x < varCount);
	assert(pY < funCount);
	Set(x, w.Sqrt() * v);
}

//------------------------------------------------------------------------------
void Jacobian::Set(int x, double v)
{
	assert(isfinite(v));

	if (v != 0)
	{
		assert(x >= 0 && x < matJ->cols());
		tp.push_back(Tpd(pY, x, v));
	}
}

//------------------------------------------------------------------------------
void Jacobian::NextFunction()
{
	pY++;
}

