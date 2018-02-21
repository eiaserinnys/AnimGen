#include "pch.h"
#include "Variable.h"

using namespace std;
using namespace Core;
using namespace Eigen;

//------------------------------------------------------------------------------
void Variable::StartUp(int count_)
{
	pY = 0;
	count = count_;
	matX.reset(new Matrix<double, Dynamic, 1>(count_));
}

//------------------------------------------------------------------------------
void Variable::CleanUp()
{
	pY = 0;
	count = 0;
	matX.reset(nullptr);
}

//------------------------------------------------------------------------------
void Variable::Begin()
{
	pY = 0;
}

//------------------------------------------------------------------------------
void Variable::End()
{
	assert(pY == count);
}

//------------------------------------------------------------------------------
void Variable::Load(Matrix4D& tx, LoadFlag::Value flag)
{
	for (int y = 0; y < 3; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			if (flag == LoadFlag::Load)
			{
				(*matX)(pY++, 0) = tx.m[y][x];
			}
			else if (flag == LoadFlag::Unload)
			{
				tx.m[y][x] = (*matX)(pY++, 0);
			}
		}
	}
}

//------------------------------------------------------------------------------
void Variable::Load(float& v, LoadFlag::Value flag)
{
	if (flag == LoadFlag::Load)
	{
		(*matX)(pY++, 0) = v;
	}
	else if (flag == LoadFlag::Unload)
	{
		v = (float) (*matX)(pY++, 0);
	}
}

//------------------------------------------------------------------------------
void Variable::Load(double& v, LoadFlag::Value flag)
{
	if (flag == LoadFlag::Load)
	{
		(*matX)(pY++, 0) = v;
	}
	else if (flag == LoadFlag::Unload)
	{
		v = (*matX)(pY++, 0);
	}
}

