#pragma once

#include "RobotCoordinate.h"

class ISpline;

class ISolutionVector {
public:
	virtual ~ISolutionVector() = 0;
	
	virtual int GetPhaseCount() const = 0;

	virtual double GetPhaseTime(int i) const = 0;
	virtual double GetLastPhaseTime() const = 0;

	virtual SolutionCoordinate& GetPhase(int i) = 0;
	virtual const SolutionCoordinate& GetPhase(int i) const = 0;

	virtual SolutionCoordinate& GetLastPhase() = 0;
	virtual const SolutionCoordinate& GetLastPhase() const = 0;

	virtual ISpline* GetCurve(int i) = 0;

	virtual int VariableCount() const = 0;

	virtual double GetVariableAt(int i) const = 0;

	virtual void SetVariableAt(int i, double v) = 0;

	virtual SolutionCoordinate At(double t) const = 0;

	virtual GeneralCoordinate GeneralAccelerationAt(double t) const = 0;

	virtual void Dump() = 0;

	virtual ISolutionVector* Clone() const = 0;

	static double Timestep();

	static ISolutionVector* BuildTest(const SolutionCoordinate& init);
};