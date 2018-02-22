#include "pch.h"
#include "SolverContext.h"

#include <WindowsUtility.h>

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

		auto& test = [&](double step)
		{
			unique_ptr<ISolutionVector> s2(solution->Clone());

			double reserved = s2->GetVariableAt(1);

			int p = 1;

			double pt = s2->GetPhaseTime(p);

			WindowsUtility::Debug(L"step %f\n", step);

			s2->SetVariableAt(1, reserved - step);
			auto ga0 = s2->GeneralAccelerationAt(pt);
			WindowsUtility::Debug(L"\tacc0 %f\n", ga0.leg[0].rot1.z);

			//s2->SetVariableAt(1, reserved + step);
			//auto ga1 = s2->GeneralAccelerationAt(pt);
			//WindowsUtility::Debug(L"\tacc1 %f\n", ga1.leg[0].rot1.z);

			double error = (0 - SquaredLength(ga0.leg[0].rot1)) / (step * 2);

			WindowsUtility::Debug(L"\terror -> %f\n", error);
		};

		test(0.000001);
		test(0.00001);

		int var = solution->VariableCount();
		int fn = 6 + 12 * solution->GetPhaseCount();

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

			error += residual.Set(Length(ga.body.first), w);
			error += residual.Set(Length(ga.body.second), w);

			error += residual.Set(Length(ga.leg[0].rot1), w);
			error += residual.Set((ga.leg[0].len1), w);
			error += residual.Set(Length(ga.leg[0].rot2), w);
			error += residual.Set((ga.leg[0].len2), w);
			error += residual.Set(Length(ga.leg[0].footRot), w);

			error += residual.Set(Length(ga.leg[1].rot1), w);
			error += residual.Set((ga.leg[1].len1), w);
			error += residual.Set(Length(ga.leg[1].rot2), w);
			error += residual.Set((ga.leg[1].len2), w);
			error += residual.Set(Length(ga.leg[1].footRot), w);
		}

		return error;
	}

	double Squared(double v)
	{
		return v * v;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian_GeneralAcceleration(
		const ISolutionVector* s,
		const Coefficient& w)
	{
		unique_ptr<ISolutionVector> s2(s->Clone());
		double step = 0.0001;

		// 초기는 어쩔 수 없다고 치고
		for (int i = 0; i < 12; ++i)
		{
			jacobian.NextFunction();
		}

		WindowsUtility::Debug(L"GeneralAcceleration\n");

		for (size_t p = 1; p < s->GetPhaseCount(); ++p)
		{
			auto coordVar = SolutionCoordinate::VariableCount();

			double* d = new double[12 * coordVar];

			// 정확하지 않은데 느려서 답이 안 나온다;;;
			// 일단은 각 페이즈 가속도는 자신의 변화만 영향을 미치게 하자
			for (int v = 0; v < coordVar; ++v)
			{
				auto& curCoord = s2->GetPhase(p);

				auto varOfs = (p - 1) * coordVar + v;

				// 원래 값을 보존
				double reserved = s2->GetVariableAt(varOfs);

				s2->SetVariableAt(varOfs, reserved - step);
				auto ga0 = s2->GeneralAccelerationAt(s->GetPhaseTime(p));

				s2->SetVariableAt(varOfs, reserved + step);
				auto ga1 = s2->GeneralAccelerationAt(s->GetPhaseTime(p));

				d[0 * coordVar + v] = (SquaredLength(ga1.body.first) - SquaredLength(ga0.body.first)) / (step * 2);
				d[1 * coordVar + v] = (SquaredLength(ga1.body.second) - SquaredLength(ga0.body.second)) / (step * 2);
				d[2 * coordVar + v] = (SquaredLength(ga1.leg[0].rot1) - SquaredLength(ga0.leg[0].rot1)) / (step * 2);
				d[3 * coordVar + v] = (Squared(ga1.leg[0].len1) - Squared(ga0.leg[0].len1)) / (step * 2);
				d[4 * coordVar + v] = (SquaredLength(ga1.leg[0].rot2) - SquaredLength(ga0.leg[0].rot2)) / (step * 2);
				d[5 * coordVar + v] = (Squared(ga1.leg[0].len2) - Squared(ga0.leg[0].len2)) / (step * 2);
				d[6 * coordVar + v] = (SquaredLength(ga1.leg[0].footRot) - SquaredLength(ga0.leg[0].footRot)) / (step * 2);
				d[7 * coordVar + v] = (SquaredLength(ga1.leg[1].rot1) - SquaredLength(ga0.leg[1].rot1)) / (step * 2);
				d[8 * coordVar + v] = (Squared(ga1.leg[1].len1) - Squared(ga0.leg[1].len1)) / (step * 2);
				d[9 * coordVar + v] = (SquaredLength(ga1.leg[1].rot2) - SquaredLength(ga0.leg[1].rot2)) / (step * 2);
				d[10 * coordVar + v] = (Squared(ga1.leg[1].len2) - Squared(ga0.leg[1].len2)) / (step * 2);
				d[11 * coordVar + v] = (SquaredLength(ga1.leg[1].footRot) - SquaredLength(ga0.leg[1].footRot)) / (step * 2);

				// 다시 원복
				s2->SetVariableAt(varOfs, reserved);

			}

			for (int f = 0; f < 12; ++f)
			{
				for (int v = 0; v < coordVar; ++v)
				{
					auto varOfs = (p - 1) * coordVar + v;

					jacobian.Set(varOfs, w, d[f * coordVar + v]);
				}
				jacobian.NextFunction();
			}

			delete[] d;
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