#pragma once

#include <memory>

#include "SolutionVector.h"
#include "RobotCoordinate.h"

#include "Variable.h"
#include "Jacobian.h"

//------------------------------------------------------------------------------
class ISolverContext {
public:
	virtual ~ISolverContext() = 0;

	virtual int VariableCount() const = 0;

	virtual ::Jacobian& Jacobian() = 0;

	virtual void LoadVariable(LoadFlag::Value flag) = 0;

	virtual double LoadResidual(bool writeDebug) = 0;

	virtual void LoadJacobian() = 0;

	virtual void CleanUp() = 0;

	static ISolverContext* Create(
		const SolutionCoordinate& start, 
		const SolutionCoordinate& dest,
		int phases);
};
