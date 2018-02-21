#pragma once

#include <memory>
#include <vector>
#include <Eigen/Sparse>

#include "Coefficient.h"

#include "SolutionVector.h"

#include "SolverHelper.h"

class ErrorFunction {
public:
	void StartUp(int funcCount);
	void CleanUp();

	//void SetDump(ISolverDump* dump) { log = dump; }

	void Begin();

	double Set(double v, const Coefficient& w);

	void End();

	Eigen::Matrix<double, Eigen::Dynamic, 1>& Raw() { return *matF; }
	const Eigen::Matrix<double, Eigen::Dynamic, 1>& Raw() const { return *matF; }

private:
	//ISolverDump* log;
	std::unique_ptr<Eigen::Matrix<double, Eigen::Dynamic, 1>> matF;
	int pY;
	int funcCount;
};