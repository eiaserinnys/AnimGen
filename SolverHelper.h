#pragma once

#include <Eigen/Sparse>

//------------------------------------------------------------------------------
inline Eigen::SparseMatrix<double>* InitializeSparseIdentity(int t)
{
	auto identity = new Eigen::SparseMatrix<double>(t, t);

	std::vector<Eigen::Triplet<double>> tp;
	for (auto i = 0; i < t; ++i) { tp.push_back(Eigen::Triplet<double>(i, i, 1)); }
	(*identity).setFromTriplets(tp.begin(), tp.end());

	return identity;
}
