#include "pch.h"
#include "Solver.h"

#include "ExponentialMap.h"

#include "Robot.h"
#include "SolutionVector.h"

#include "Variable.h"
#include "Coefficient.h"
#include "Residual.h"
#include "Jacobian.h"

#include "SolverHelper.h"

using namespace std;
using namespace Core;
using namespace Eigen;

#define DUMP 0

//------------------------------------------------------------------------------
struct SolverContext
{
	Coefficient one;

	SolutionVector solution;
	SolutionCoordinate dest;

	Variable variables;
	Residual errorFunc;
	Jacobian jacobian;

	//------------------------------------------------------------------------------
	SolverContext()
		: one(1)
	{
	}

	//------------------------------------------------------------------------------
	int VariableCount() const
	{ 
		return solution.VariableCount();
	}

	//------------------------------------------------------------------------------
	void LoadX(LoadFlag::Value flag)
	{
		variables.Begin();

		auto nodeCount = solution.GetPhaseCount();

		// 0번 = 초기 위치는 수정하지 않으므로 스킵한다
		for (size_t i = 1; i < nodeCount; i++)
		{
			auto& sc = solution.GetPhase(i);
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

		errorFunc.Begin();

		error += LoadResidual_DestinationTask(solution, dest, one, writeDebug);

		error += LoadResidual_GeneralAcceleration(solution, one, writeDebug);

		errorFunc.End();

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian()
	{
		jacobian.Begin();

		LoadJacobian_DestinationTask(solution, dest, one);

		//LoadJacobian_GeneralAcceleration(solution, one);

		jacobian.End();
	}

	//------------------------------------------------------------------------------
	double LoadResidual_DestinationTask(
		const SolutionVector& s,
		const SolutionCoordinate& d,
		const Coefficient& w,
		bool writeDebug)
	{
		double error = 0;

		const auto& last = s.GetLastPhase();

		auto& setRot = [&](const Vector3D& dest, const Vector3D& cur) -> double
		{
			auto nearRot = ExponentialMap::GetNearRotation(dest, cur);
			return errorFunc.Set(Distance(dest, nearRot), w);
		};

		error += errorFunc.Set(Distance(last.body.first, d.body.first), w);
		error += setRot(last.body.second, d.body.second);

		error += errorFunc.Set(Distance(last.foot[0].first, d.foot[0].first), w);
		error += setRot(last.foot[0].second, d.foot[0].second);

		error += errorFunc.Set(Distance(last.foot[1].first, d.foot[1].first), w);
		error += setRot(last.foot[1].second, d.foot[1].second);

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian_DestinationTask(
		const SolutionVector& s,
		const SolutionCoordinate& d,
		const Coefficient& w)
	{
		auto s2 = s;

		auto& taskError0 = [](const SolutionVector& cur, const SolutionCoordinate& dest)
		{
			const auto& last = cur.GetLastPhase();
			double e = Distance(last.body.first, dest.body.first);
			return e * e;
		};

		double step = 0.0001;

		for (int i = 0; i < s.VariableCount(); ++i)
		{
			double reserved = s2.GetVariableAt(i);

			s2.SetVariableAt(i, reserved - step);
			double e0 = taskError0(s2, d);

			s2.SetVariableAt(i, reserved + step);
			double e1 = taskError0(s2, d);

			double j = (e1 - e0) / (step * 2);

			jacobian.Set(i, w, j);
		}

		//jacobian.Set(0, w, );
	}

	//------------------------------------------------------------------------------
	double LoadResidual_GeneralAcceleration(
		const SolutionVector& s,
		const Coefficient& w,
		bool writeDebug)
	{
		double error = 0;

		for (size_t i = 0; i < s.GetPhaseCount(); ++i)
		{
			auto ga = s.GeneralAccelerationAt(s.GetPhaseTime(i));
			error += errorFunc.Set(sqrt(ga.SquaredLength()), w);
		}

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian_GeneralAcceleration(
		const SolutionVector& s,
		const Coefficient& w)
	{
		auto s2 = s;

		double step = 0.0001;

		for (size_t i = 0; i < s.GetPhaseCount(); ++i)
		{
			for (int j = 0; j < s.VariableCount(); ++j)
			{
				double reserved = s2.GetVariableAt(j);

				s2.SetVariableAt(j, reserved - step);
				auto ga0 = s2.GeneralAccelerationAt(s.GetPhaseTime(i));
				double e0 = ga0.SquaredLength();

				s2.SetVariableAt(i, reserved + step);
				auto ga1 = s2.GeneralAccelerationAt(s.GetPhaseTime(i));
				double e1 = ga1.SquaredLength();

				double v = (e1 - e0) / (step * 2);

				jacobian.Set(i, w, v);
			}

			jacobian.NextFunction();
		}
	}

	void CleanUp() {}
};

//------------------------------------------------------------------------------
class Solver : public ISolver {
public:
	double fK, fK1;
	double nu;
	double lambda;

	unique_ptr<SolverContext> context;

	unique_ptr<SparseMatrixD> identity;

public:
	//------------------------------------------------------------------------------
	void Begin()
	{
		nu = 2;

		identity.reset(InitializeSparseIdentity(context->VariableCount()));

		context->LoadX(LoadFlag::Load);

		// 이제 풀자
		fK = context->LoadResidual(true);

#	if DUMP
		dump->WriteLine(
			ISolverDump::Console | ISolverDump::Debug,
			L" -> Initial");
#	endif

		fK1 = fK;

		context->LoadJacobian();

#if 0
		// Levenberg-Marquardt
		lambda = InitialLambda(context->Jacobian().RawJtJ());
#endif
	}

	//------------------------------------------------------------------------------
	typename Result::Value SolveStep()
	{
		auto result = SolveStep_();
#if 0

		// 이번에는 스텝마다 매번 결과를 받는다
		switch (result) {
		case Result::Solved:
		case Result::StepAccepted:
			context->WriteSolution();
			break;
		}
#endif

		return result;
	}

	//------------------------------------------------------------------------------
	Result::Value SolveStep_()
	{
#if 0
		double error1 = 1e-6;	// 총 에러가 이 수치 이하게 되면 정지
		double error2 = 1e-6;

		context->Jacobian().RawJtJ() += lambda * (*identity);

		Eigen::Matrix<double, Eigen::Dynamic, 1> g =
			context->Jacobian().RawJ().transpose() * 
			context->ErrorFunc().Raw();

		Eigen::Matrix<double, Eigen::Dynamic, 1> mg = -g;

		Eigen::SimplicialLLT<Eigen::SparseMatrixD> solver;

		auto& intm = solver.compute(context->Jacobian().RawJtJ());
		if (intm.info() != Eigen::Success) { return Result::Unsolvable; }

		Eigen::Matrix<double, Eigen::Dynamic, 1> h = intm.solve(mg);
		if (intm.info() != Eigen::Success) { return Result::Unsolvable; }

		Eigen::Matrix<double, Eigen::Dynamic, 1> prevX = context->Variables().Raw();

		context->Variables().Raw() += h;

		context->LoadX(LoadFlag::Unload);

		fK1 = context->BuildF(true);

		double actualGain = fK - fK1;
		double predictedGain = 0.5 * h.transpose() * (lambda * h - g);
		double rho = actualGain / predictedGain;

		//double solvedError = 1e-7;
		//double relaxError = 1e-5;
		double solvedError = 1e-5;
		double relaxError = 1e-4;

		if (rho >= 0)
		{
#if 0
			dump->Write(ISolverDump::Move, L"Accepted");
			for (size_t i = 0; i < h.rows(); ++i) { dump->Write(ISolverDump::Move, L"\t%f", h(i, 0)); }
			dump->WriteLine(ISolverDump::Move);
#endif

			auto factor = max(1 / 3.0, 1 - pow(2 * rho - 1, 3));
			lambda = lambda * factor;
			nu = 2;

#if 0
			dump->WriteLine(
				ISolverDump::Console | ISolverDump::Debug,
				L"lambda=%f nu=%f rho=%f -> Accepted", lambda, nu, rho);
#endif

			if (abs(fK - fK1) < solvedError * (1 + fK))
			{
				// 완전 수렴
#if 0
				dump->WriteLine(
					ISolverDump::Console | ISolverDump::Debug,
					L"Solved\n");
#endif
				return Result::Solved;
			}

			// accept the step
			context->BuildJacobian();

			fK = fK1;

			return Result::StepAccepted;
		}
		else
		{
#if 0 
			dump->Write(ISolverDump::Move, L"Rejected");
			for (size_t i = 0; i < h.rows(); ++i) { dump->Write(ISolverDump::Move, L"\t%f", h(i, 0)); }
			dump->WriteLine(ISolverDump::Move);

			Reject(rho, prevX);
#endif

			if (!isfinite(lambda) || !isfinite(nu))
			{
#if 0 
				dump->WriteLine(
					ISolverDump::Console | ISolverDump::Debug,
					L"Unsolvable\n");
#endif
				return Result::Unsolvable;
			}

			return Result::StepRejected;
		}
#endif
		return Result::Unsolvable;
	}

	//------------------------------------------------------------------------------
	void Reject(
		double rho,
		const Eigen::Matrix<double, Eigen::Dynamic, 1>& prevX)
	{
#if 0
		context->Variables().Raw() = prevX;

		context->LoadX(LoadFlag::Unload);

		context->BuildF(false);

		lambda = lambda * nu;
		nu = nu * 2;

#if 0
		dump->WriteLine(
			ISolverDump::Console | ISolverDump::Debug,
			L"lambda=%f nu=%f rho=%f -> Rejected", lambda, nu, rho);
#endif
#endif
	}

	//------------------------------------------------------------------------------
	void End()
	{
#if 0
		context->CleanUp();

		identity.reset(nullptr);
#endif
	}
};

ISolver::~ISolver() = default;

ISolver* ISolver::Create()
{ return new Solver; }