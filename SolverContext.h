#pragma once

#include <memory>

#include "SolutionVector.h"

#include "Variable.h"
#include "Residual.h"
#include "Jacobian.h"

class ISolverLog;

//------------------------------------------------------------------------------
class ISolverContext {
public:
	virtual ~ISolverContext() = 0;

	virtual int VariableCount() const = 0;

	virtual ::Jacobian& Jacobian() = 0;

	virtual ::Residual& Residual() = 0;

	virtual ::Variable& Variable() = 0;

	virtual void LoadVariable(LoadFlag::Value flag) = 0;

	virtual double LoadResidual(bool writeDebug) = 0;

	virtual void LoadJacobian() = 0;

	virtual void SetLog(ISolverLog* log) = 0;

	virtual void CleanUp() = 0;

	virtual ISolutionVector* Solution() = 0;

	static ISolverContext* Create(
		const SolutionCoordinate& start, 
		const SolutionCoordinate& dest,
		int phases);
};
