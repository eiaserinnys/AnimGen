#include "pch.h"
#include "Solver.h"

#include "Vector.h"
#include "SolverHelper.h"

using namespace std;

#if 0
//------------------------------------------------------------------------------
struct SolverContext
{
	int VariableCount() const { return 1; }

	void CleanUp() {}
};

//------------------------------------------------------------------------------
class Solver : public ISolver {
public:
	double fK, fK1;
	double nu;
	double lambda;

	unique_ptr<SolverContext> context;

	unique_ptr<SparseMatrix<double>> identity;

public:
	//------------------------------------------------------------------------------
	void Begin()
	{
		nu = 2;

		identity.reset(InitializeSparseIdentity(context->VariableCount()));

		context->LoadX(LoadFlag::Load);

		// 이제 풀자
		fK = context->BuildF(true);
		dump->WriteLine(
			ISolverDump::Console | ISolverDump::Debug,
			L" -> Initial");

		fK1 = fK;

		context->BuildJacobian();

		// Levenberg-Marquardt
		lambda = InitialLambda(context->Jacobian().RawJtJ());
	}

	//------------------------------------------------------------------------------
	typename Result::Value SolveStep()
	{
		auto result = SolveStep_();

		// 이번에는 스텝마다 매번 결과를 받는다
		switch (result) {
		case Result::Solved:
		case Result::StepAccepted:
		case Result::StepAccepted_CoeffHalved:
		case Result::StepAccepted_ConfReseted:
			context->WriteSolution();
			break;
		}

		return result;
	}

	//------------------------------------------------------------------------------
	Result::Value SolveStep_()
	{
		double error1 = 1e-6;	// 총 에러가 이 수치 이하게 되면 정지
		double error2 = 1e-6;

		context->Jacobian().RawJtJ() += lambda * (*identity);

		Eigen::Matrix<double, Eigen::Dynamic, 1> g =
			context->Jacobian().RawJ().transpose() * 
			context->ErrorFunc().Raw();

		Eigen::Matrix<double, Eigen::Dynamic, 1> mg = -g;

		Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> solver;

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
	}

	//------------------------------------------------------------------------------
	void Reject(
		double rho,
		const Eigen::Matrix<double, Eigen::Dynamic, 1>& prevX)
	{
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
	}

	//------------------------------------------------------------------------------
	void End()
	{
		context->CleanUp();

		identity.reset(nullptr);
	}
};

ISolver* ISolver::Create()
{ return new Solver; }

#endif