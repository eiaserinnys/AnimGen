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

	Eigen::SparseMatrixD& RawJ() { return *matJ; }
	const Eigen::SparseMatrixD& RawJ() const { return *matJ; }

	Eigen::SparseMatrixD& RawJtJ() { return *matJtJ; }
	const Eigen::SparseMatrixD& RawJtJ() const { return *matJtJ; }

private:
	void Set(int x, double v);

	std::vector<Eigen::TripletD> tp;

	std::unique_ptr<Eigen::SparseMatrixD> matJ;
	std::unique_ptr<Eigen::SparseMatrixD> matJtJ;

	int pY;
	int funCount;
	int varCount;
};