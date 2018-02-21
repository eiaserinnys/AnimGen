#include "pch.h"
#include "SolverHelper.h"

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
