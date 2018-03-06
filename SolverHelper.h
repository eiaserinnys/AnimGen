#pragma once

#include <Eigen/Sparse>

namespace Eigen 
{ 
	typedef SparseMatrix<double> SparseMatrixD; 
	typedef Triplet<double> TripletD;
}

//------------------------------------------------------------------------------
Eigen::SparseMatrix<double>* InitializeSparseIdentity(int t);

double InitialLambda(Eigen::SparseMatrix<double>& JtJ);
