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
#include "SolverLog.h"

using namespace std;
//using namespace Core;
using namespace Eigen;

#define DUMP 1

//------------------------------------------------------------------------------
// Levenberg-Marquardt Non Linear Optimizer
class Solver : public ISolver {
public:
	double fK, fK1;
	double nu;
	double lambda;

	unique_ptr<ISolverContext> context;

	unique_ptr<SparseMatrixD> identity;

	ISolverLog* log = nullptr;
	bool ownLog = false;

public:
	Solver(
		const SolutionCoordinate& start,
		const SolutionCoordinate& dest,
		int phases)
	{
		context.reset(ISolverContext::Create(start, dest, phases));
	}

	~Solver()
	{
		End();
	}

	//------------------------------------------------------------------------------
	template <typename... Arguments>
	void Write(const Arguments&... args)
	{ if (log != nullptr) { log->Write(args...); } }

	template <typename... Arguments>
	void WriteLine(const Arguments&... args)
	{ if (log != nullptr) { log->WriteLine(args...); } }

	virtual ISolutionVector* Solution()
	{
		return context->Solution();
	}

	//------------------------------------------------------------------------------
	void Begin(ISolverLog* log, bool ownLog)
	{
		End();

		context->SetLog(log);

		this->log = log;
		this->ownLog = ownLog;

		nu = 2;

		identity.reset(InitializeSparseIdentity(context->VariableCount()));

		context->LoadVariable(LoadFlag::Load);

		// 이제 풀자
		fK = context->LoadResidual(true);

#		if DUMP
		WriteLine(
			ISolverLog::Console | ISolverLog::Debug | ISolverLog::Residual,
			L" -> Initial");
#		endif

		fK1 = fK;

		context->LoadJacobian();

		lambda = InitialLambda(context->Jacobian().RawJtJ());

		DumpSolution();
	}

	//------------------------------------------------------------------------------
	void DumpSolution()
	{
		auto sol = context->Solution();
		for (int i = 0; i < sol->GetPhaseCount(); ++i)
		{
			auto& phase = sol->GetPhase(i);

			WriteLine(
				ISolverLog::Console | ISolverLog::Debug,
				L"%02d - (%.3f,%.3f,%.3f)\t",
				i,
				phase.body.first.x,
				phase.body.first.y,
				phase.body.first.z);
		}
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
		double error1 = 1e-6;	// 총 에러가 이 수치 이하게 되면 정지
		double error2 = 1e-6;

		context->Jacobian().RawJtJ() += lambda * (*identity);

		Matrix<double, Dynamic, 1> g =
			context->Jacobian().RawJ().transpose() * 
			context->Residual().Raw();

		Matrix<double, Dynamic, 1> mg = -g;

		SimplicialLLT<SparseMatrixD> solver;

		auto& intm = solver.compute(context->Jacobian().RawJtJ());
		if (intm.info() != Success) { return Result::Unsolvable; }

		Matrix<double, Dynamic, 1> h = intm.solve(mg);
		if (intm.info() != Success) { return Result::Unsolvable; }

		Matrix<double, Dynamic, 1> prevX = context->Variable().Raw();

		context->Variable().Raw() += h;

		context->LoadVariable(LoadFlag::Unload);

		fK1 = context->LoadResidual(true);

		double actualGain = fK - fK1;
		double predictedGain = 0.5 * h.transpose() * (lambda * h - g);
		double rho = actualGain / predictedGain;

		//double solvedError = 1e-7;
		//double relaxError = 1e-5;
		double solvedError = 1e-5;
		double relaxError = 1e-4;

		if (rho >= 0)
		{
			auto factor = max(1 / 3.0, 1 - pow(2 * rho - 1, 3));
			lambda = lambda * factor;
			nu = 2;

#			if DUMP
			{
				WriteLine(
					ISolverLog::Console | ISolverLog::Debug,
					L"lambda=%f nu=%f rho=%f -> Accepted", lambda, nu, rho);

				DumpSolution();

				Write(ISolverLog::Move | ISolverLog::Residual, L"Accepted");
				for (size_t i = 0; i < h.rows(); ++i) { Write(ISolverLog::Move, L"\t%f", h(i, 0)); }
				WriteLine(ISolverLog::Move | ISolverLog::Residual);
			}
#			endif

			if (abs(fK - fK1) < solvedError * (1 + fK))
			{
				// 완전 수렴
#				if DUMP
				WriteLine(
					ISolverLog::Console | ISolverLog::Debug,
					L"Solved\n");
#				endif
				return Result::Solved;
			}

			// accept the step
			context->LoadJacobian();

			fK = fK1;

			return Result::StepAccepted;
		}
		else
		{
			return Reject(rho, h, prevX);
		}
		return Result::Unsolvable;
	}

	//------------------------------------------------------------------------------
	Result::Value Reject(
		double rho,
		Matrix<double, Dynamic, 1>& h, 
		const Matrix<double, Dynamic, 1>& prevX)
	{
		auto oldLambda = lambda;
		auto oldNu = nu;

		context->Variable().Raw() = prevX;

		context->LoadVariable(LoadFlag::Unload);

		context->LoadResidual(false);

		lambda = lambda * nu;

		nu = nu * 2;

		if (!isfinite(lambda) || !isfinite(nu))
		{
#			if DUMP
			log->WriteLine(
				ISolverLog::Console | ISolverLog::Debug,
				L"Unsolvable\n");
#			endif
			return Result::Unsolvable;
		}
		else
		{
#			if DUMP
			log->WriteLine(
				ISolverLog::Console | ISolverLog::Debug,
				L"lambda=%f nu=%f rho=%f -> Rejected", oldLambda, oldNu, rho);

			Write(ISolverLog::Move | ISolverLog::Residual, L"Rejected");
			for (size_t i = 0; i < h.rows(); ++i) { Write(ISolverLog::Move, L"\t%f", h(i, 0)); }
			WriteLine(ISolverLog::Move | ISolverLog::Residual);
#			endif
			return Result::StepRejected;
		}
	}

	//------------------------------------------------------------------------------
	void End()
	{
		context->CleanUp();
		identity.reset(nullptr);

		if (ownLog)
		{
			delete log;
		}
		log = nullptr;
	}
};

ISolver::~ISolver() = default;

ISolver* ISolver::Create(
	const SolutionCoordinate& start,
	const SolutionCoordinate& dest,
	int phases)
{ return new Solver(start, dest, phases); }