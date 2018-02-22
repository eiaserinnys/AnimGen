#include "pch.h"
#include "SolverHelper.h"

#undef min

using namespace std;
using namespace Eigen;

//------------------------------------------------------------------------------
SparseMatrixD* InitializeSparseIdentity(int t)
{
	auto identity = new SparseMatrixD(t, t);

	vector<Eigen::Triplet<double>> tp;
	for (auto i = 0; i < t; ++i) { tp.push_back(Eigen::Triplet<double>(i, i, 1)); }
	(*identity).setFromTriplets(tp.begin(), tp.end());

	return identity;
}

//------------------------------------------------------------------------------
double InitialLambda(Eigen::SparseMatrix<double>& JtJ)
{
	double tau = 1e-6;

	double r = std::numeric_limits<double>::min();

	for (int i = 0; i < JtJ.cols(); ++i)
	{
		if (r < JtJ.coeffRef(i, i))
		{
			r = JtJ.coeffRef(i, i);
		}
	}

	return tau * r;
}
