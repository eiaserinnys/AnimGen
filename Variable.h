#pragma once

#include <memory>
#include <Eigen/Sparse>
#include <DirectXMath.h>

#include "Vector.h"
#include "Matrix.h"
#include "SolverHelper.h"

struct LoadFlag
{
	enum Value
	{
		Load,
		Unload,
	};
};

class Variable {
public:
	void StartUp(int count);
	void CleanUp();

	void Begin();

	void Load(Core::Matrix4D& tx, LoadFlag::Value flag);
	void Load(double& v, LoadFlag::Value flag);
	void Load(float& v, LoadFlag::Value flag);

	template <typename V, int N>
	void Load(Core::VectorT<V, N>& v, LoadFlag::Value flag)
	{
		if (flag == LoadFlag::Load)
		{
			for (int i = 0; i < N; ++i)
			{
				(*matX)(pY++, 0) = v.m[i];
			}
		}
		else if (flag == LoadFlag::Unload)
		{
			for (int i = 0; i < N; ++i)
			{
				v.m[i] = (*matX)(pY++, 0);
			}
		}
	}

	void End();

	Eigen::Matrix<double, Eigen::Dynamic, 1>& Raw() { return *matX; }
	const Eigen::Matrix<double, Eigen::Dynamic, 1>& Raw() const { return *matX; }

private:
	int pY = 0;
	int count = 0;
	std::unique_ptr<Eigen::Matrix<double, Eigen::Dynamic, 1>> matX;
};