#pragma once

#include <memory>
#include <vector>
#include <Eigen/Sparse>
#include <DirectXMath.h>

#include "SolverHelper.h"

class Jacobian {
public:
	void StartUp(int fn, int var);
	void CleanUp();

	void Begin();

	void NextFunction();

	void Set(int x, const Coefficient& w, double v);

	void SetRotation(
		DeformGraph* graph,
		const MatrixD* nodeTxD,
		const Coefficient& w_rot,
		const std::function<int(int, int, int)>& getOfs);
	void SetRotation(
		const MatrixD& tx,
		int index,
		const Coefficient& w_rot,
		const std::function<int(int, int, int)>& getOfs);

	void SetRotation_Null(DeformGraph* graph);

	void SetRegularization(
		DeformGraph* graph,
		const MatrixD* tx,
		const Coefficient& w_reg,
		const std::function<int(int, int, int)>& getOfs);

	void SetRegularization_Null(DeformGraph* graph);

	void SetConfidence(
		double* conf,
		int nodeCount,
		const Coefficient& w_conf,
		const std::function<int(int)>& getOfs);

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