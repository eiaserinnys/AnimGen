#pragma once

#include <memory>
#include <vector>
#include <Eigen/Sparse>

#include "Coefficient.h"
#include "SolverHelper.h"

class Jacobian {
public:
	void StartUp(int fn, int var);
	void CleanUp();

	void Begin();

	void NextFunction();

	void Set(int x, const Coefficient& w, double v);

	void End();

	Eigen::SparseMatrix<double>& RawJ() { return *matJ; }
	const Eigen::SparseMatrix<double>& RawJ() const { return *matJ; }

	Eigen::SparseMatrix<double>& RawJtJ() { return *matJtJ; }
	const Eigen::SparseMatrix<double>& RawJtJ() const { return *matJtJ; }

private:
	void Set(int x, double v);

	typedef Eigen::Triplet<double> Tpd;
	std::vector<Tpd> tp;

	std::unique_ptr<Eigen::SparseMatrix<double>> matJ;
	std::unique_ptr<Eigen::SparseMatrix<double>> matJtJ;

	int pY;
	int funCount;
	int varCount;
};