#pragma once

#include <Eigen/Sparse>

namespace Eigen { typedef SparseMatrix<double> SparseMatrixD; }

//------------------------------------------------------------------------------
Eigen::SparseMatrix<double>* InitializeSparseIdentity(int t);
