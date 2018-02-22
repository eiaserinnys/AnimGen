#include "pch.h"
#include "SolverContext.h"

#include "ExponentialMap.h"

#include "Coefficient.h"
#include "Variable.h"
#include "Residual.h"

using namespace std;
using namespace Core;

//------------------------------------------------------------------------------
class SolverContext : public ISolverContext  {
public:
	std::unique_ptr<ISolutionVector> solution;
	SolutionCoordinate dest;

	Coefficient one;
	Variable variables;
	Residual residual;
	::Jacobian jacobian;

	//------------------------------------------------------------------------------
	SolverContext(
		const SolutionCoordinate& start, 
		const SolutionCoordinate& dest,
		int phases)
		: one(1)
		, dest(dest)
	{
		solution.reset(ISolutionVector::Create(start, phases));

		int var = solution->VariableCount();
		int fn = 6 + solution->GetPhaseCount();

		variables.StartUp(var);
		residual.StartUp(fn);
		jacobian.StartUp(fn, var);
	}

	//------------------------------------------------------------------------------
	int VariableCount() const { return solution->VariableCount(); }

	//------------------------------------------------------------------------------
	::Jacobian& Jacobian() { return jacobian; }

	//------------------------------------------------------------------------------
	void LoadVariable(LoadFlag::Value flag)
	{
		variables.Begin();

		auto nodeCount = solution->GetPhaseCount();

		// 0번 = 초기 위치는 수정하지 않으므로 스킵한다
		for (size_t i = 1; i < nodeCount; i++)
		{
			auto& sc = solution->GetPhase(i);
			variables.Load(sc.body.first, flag);
			variables.Load(sc.body.second, flag);
			variables.Load(sc.foot[0].first, flag);
			variables.Load(sc.foot[0].second, flag);
			variables.Load(sc.foot[1].first, flag);
			variables.Load(sc.foot[1].second, flag);
		}

		variables.End();
	}

	//------------------------------------------------------------------------------
	double LoadResidual(bool writeDebug)
	{
		double error = 0;

		residual.Begin();

		error += LoadResidual_DestinationTask(solution.get(), dest, one, writeDebug);

		error += LoadResidual_GeneralAcceleration(solution.get(), one, writeDebug);

		residual.End();

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian()
	{
		jacobian.Begin();

		LoadJacobian_DestinationTask(solution.get(), dest, one);

		LoadJacobian_GeneralAcceleration(solution.get(), one);

		jacobian.End();
	}

	//------------------------------------------------------------------------------
	double LoadResidual_DestinationTask(
		const ISolutionVector* s,
		const SolutionCoordinate& d,
		const Coefficient& w,
		bool writeDebug)
	{
		double error = 0;

		const auto& last = s->GetLastPhase();

		auto& setRot = [&](const Vector3D& dest, const Vector3D& cur) -> double
		{
			auto nearRot = ExponentialMap::GetNearRotation(dest, cur);
			return residual.Set(Distance(dest, nearRot), w);
		};

		error += residual.Set(Distance(last.body.first, d.body.first), w);
		error += setRot(last.body.second, d.body.second);

		error += residual.Set(Distance(last.foot[0].first, d.foot[0].first), w);
		error += setRot(last.foot[0].second, d.foot[0].second);

		error += residual.Set(Distance(last.foot[1].first, d.foot[1].first), w);
		error += setRot(last.foot[1].second, d.foot[1].second);

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian_DestinationTask(
		const ISolutionVector* s,
		const SolutionCoordinate& d,
		const Coefficient& w)
	{
		const auto& last = s->GetLastPhase();

		unique_ptr<ISolutionVector> s2(s->Clone());
		const auto& last2 = s2->GetLastPhase();
		double step = 0.0001;

		int varPivot = (s->GetPhaseCount() - 2) * SolutionCoordinate::VariableCount();

		// body pos diff
		jacobian.Set(varPivot++, w, last.body.first.m[0] - d.body.first.m[0]);
		jacobian.Set(varPivot++, w, last.body.first.m[1] - d.body.first.m[1]);
		jacobian.Set(varPivot++, w, last.body.first.m[2] - d.body.first.m[2]);
		jacobian.NextFunction();

		// body rot diff
		for (int ch = 0; ch < 3; ++ch)
		{
			double reserved = s2->GetVariableAt(varPivot);

			s2->SetVariableAt(varPivot, reserved - step);
			auto rotm = ExponentialMap::GetNearRotation(dest.body.second, last2.body.second);

			s2->SetVariableAt(varPivot, reserved + step);
			auto rotp = ExponentialMap::GetNearRotation(dest.body.second, last2.body.second);

			double errorm = 0.5 * SquaredDistance(dest.body.second, rotm);
			double errorp = 0.5 * SquaredDistance(dest.body.second, rotp);
			double derivative = (errorp - errorm) / (2 * step);

			s2->SetVariableAt(varPivot, reserved);

			jacobian.Set(varPivot++, w, derivative);
		}
		jacobian.NextFunction();

		// left foot pos diff
		jacobian.Set(varPivot++, w, last.foot[0].first.m[0] - d.foot[0].first.m[0]);
		jacobian.Set(varPivot++, w, last.foot[0].first.m[1] - d.foot[0].first.m[1]);
		jacobian.Set(varPivot++, w, last.foot[0].first.m[2] - d.foot[0].first.m[2]);
		jacobian.NextFunction();

		// left foot rot diff
		for (int ch = 0; ch < 3; ++ch)
		{
			double reserved = s2->GetVariableAt(varPivot);

			s2->SetVariableAt(varPivot, reserved - step);
			auto rotm = ExponentialMap::GetNearRotation(dest.foot[0].second, last2.foot[0].second);

			s2->SetVariableAt(varPivot, reserved + step);
			auto rotp = ExponentialMap::GetNearRotation(dest.foot[0].second, last2.foot[0].second);

			double errorm = 0.5 * SquaredDistance(dest.foot[0].second, rotm);
			double errorp = 0.5 * SquaredDistance(dest.foot[0].second, rotp);
			double derivative = (errorp - errorm) / (2 * step);

			s2->SetVariableAt(varPivot, reserved);

			jacobian.Set(varPivot++, w, derivative);
		}
		jacobian.NextFunction();

		// right foot pos diff
		jacobian.Set(varPivot++, w, last.foot[1].first.m[0] - d.foot[1].first.m[0]);
		jacobian.Set(varPivot++, w, last.foot[1].first.m[1] - d.foot[1].first.m[1]);
		jacobian.Set(varPivot++, w, last.foot[1].first.m[2] - d.foot[1].first.m[2]);
		jacobian.NextFunction();

		// right foot rot diff
		for (int ch = 0; ch < 3; ++ch)
		{
			double reserved = s2->GetVariableAt(varPivot);

			s2->SetVariableAt(varPivot, reserved - step);
			auto rotm = ExponentialMap::GetNearRotation(dest.foot[1].second, last2.foot[1].second);

			s2->SetVariableAt(varPivot, reserved + step);
			auto rotp = ExponentialMap::GetNearRotation(dest.foot[1].second, last2.foot[1].second);

			double errorm = 0.5 * SquaredDistance(dest.foot[1].second, rotm);
			double errorp = 0.5 * SquaredDistance(dest.foot[1].second, rotp);
			double derivative = (errorp - errorm) / (2 * step);

			s2->SetVariableAt(varPivot, reserved);

			jacobian.Set(varPivot++, w, derivative);
		}
		jacobian.NextFunction();
	}

	//------------------------------------------------------------------------------
	double LoadResidual_GeneralAcceleration(
		const ISolutionVector* s,
		const Coefficient& w,
		bool writeDebug)
	{
		double error = 0;

		for (size_t i = 0; i < s->GetPhaseCount(); ++i)
		{
			auto ga = s->GeneralAccelerationAt(s->GetPhaseTime(i));
			error += residual.Set(sqrt(ga.SquaredLength()), w);
		}

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian_GeneralAcceleration(
		const ISolutionVector* s,
		const Coefficient& w)
	{
		unique_ptr<ISolutionVector> s2(s->Clone());
		double step = 0.0001;

		// 초기는 어쩔 수 없다고 치고
		jacobian.NextFunction();

		for (size_t i = 1; i < s->GetPhaseCount(); ++i)
		{
			// 정확하지 않은데 느려서 답이 안 나온다;;;
			// 일단은 각 페이즈 가속도는 자신의 변화만 영향을 미치게 하자
			//for (int j = 0; j < s->VariableCount(); ++j)
			for (int j = 0; j < SolutionCoordinate::VariableCount(); ++j)
			{
				auto var = (i - 1) * SolutionCoordinate::VariableCount() + j;

				// 원래 값을 보존
				double reserved = s2->GetVariableAt(var);

				s2->SetVariableAt(var, reserved - step);
				auto ga0 = s2->GeneralAccelerationAt(s->GetPhaseTime(i));
				double e0 = ga0.SquaredLength();

				s2->SetVariableAt(var, reserved + step);
				auto ga1 = s2->GeneralAccelerationAt(s->GetPhaseTime(i));
				double e1 = ga1.SquaredLength();

				double v = (e1 - e0) / (step * 2);

				jacobian.Set(var, w, v);

				// 다시 원복
				s2->SetVariableAt(var, reserved);
			}

			jacobian.NextFunction();
		}
	}

	void CleanUp() {}
};

ISolverContext::~ISolverContext() = default;

ISolverContext* ISolverContext::Create(
	const SolutionCoordinate& start, 
	const SolutionCoordinate& dest,
	int phases)
{ return new SolverContext(start, dest, phases); }