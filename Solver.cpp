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
#include "SolverContext.h"

using namespace std;
using namespace Core;
using namespace Eigen;

#define DUMP 0

//------------------------------------------------------------------------------
class Solver : public ISolver {
public:
	double fK, fK1;
	double nu;
	double lambda;

	unique_ptr<ISolverContext> context;

	unique_ptr<SparseMatrixD> identity;

public:
	Solver(
		const SolutionCoordinate& start,
		const SolutionCoordinate& dest,
		int phases)
	{
		context.reset(ISolverContext::Create(start, dest, phases));
	}

	//------------------------------------------------------------------------------
	void Begin()
	{
		nu = 2;

		identity.reset(InitializeSparseIdentity(context->VariableCount()));

		context->LoadVariable(LoadFlag::Load);

		// 이제 풀자
		fK = context->LoadResidual(true);

#	if DUMP
		dump->WriteLine(
			ISolverDump::Console | ISolverDump::Debug,
			L" -> Initial");
#	endif

		fK1 = fK;

		context->LoadJacobian();

		// Levenberg-Marquardt
		lambda = InitialLambda(context->Jacobian().RawJtJ());
	}

	//------------------------------------------------------------------------------
	typename Result::Value SolveStep()
	{
		auto result = SolveStep_();

#		if 0
		// 이번에는 스텝마다 매번 결과를 받는다
		switch (result) {
		case Result::Solved:
		case Result::StepAccepted:
			context->WriteSolution();
			break;
		}
#		endif

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

ISolver* ISolver::Create(
	const SolutionCoordinate& start,
	const SolutionCoordinate& dest,
	int phases)
{ return new Solver(start, dest, phases); }