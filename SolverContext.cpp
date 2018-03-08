#include "pch.h"
#include "SolverContext.h"

#include <Utility.h>
#include <WindowsUtility.h>

#include "ExponentialMap.h"

#include "Coefficient.h"
#include "Variable.h"
#include "Residual.h"

#include "SolverLog.h"

#include "GeneralizedAccelerationCalculator.h"
#include "SolutionCoordinate.h"

using namespace std;
using namespace Core;

//------------------------------------------------------------------------------
class SolverContext : public ISolverContext  {
public:
	std::unique_ptr<ISolutionVector> solution;
	SolutionCoordinate dest;

	Coefficient one;
	Coefficient task;

	::Variable variables;
	::Residual residual;
	::Jacobian jacobian;

	ISolverLog* log = nullptr;

	//------------------------------------------------------------------------------
	SolverContext(
		const SolutionCoordinate& start, 
		const SolutionCoordinate& dest,
		int phases)
		: one(1)
		, task(100)
		, dest(dest)
	{
		solution.reset(ISolutionVector::Create(start, phases));

		int var = solution->VariableCount();
		int fn = 6 + 12 * solution->GetPhaseCount();

		variables.StartUp(var);
		residual.StartUp(fn);
		jacobian.StartUp(fn, var);
	}

	//------------------------------------------------------------------------------
	void SetLog(ISolverLog* log) { this->log = log; }

	//------------------------------------------------------------------------------
	int VariableCount() const { return solution->VariableCount(); }

	//------------------------------------------------------------------------------
	::Jacobian& Jacobian() { return jacobian; }
	::Residual& Residual() { return residual; }
	::Variable& Variable() { return variables; }

	ISolutionVector* Solution()
	{
		return solution.get();
	}

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

		if (flag == LoadFlag::Unload)
		{
			solution->UpdateSpline();
		}
	}

	//------------------------------------------------------------------------------
	double LoadResidual(bool writeDebug)
	{
		double error = 0;

		residual.Begin();

		error += LoadResidual_DestinationTask(solution.get(), dest, task, writeDebug);

		error += LoadResidual_GeneralAcceleration(solution.get(), one, writeDebug);

		residual.End();

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian()
	{
		jacobian.Begin();

		LoadJacobian_DestinationTask(solution.get(), dest, task);

		LoadJacobian_GeneralAcceleration(solution.get(), one);

		jacobian.End();

		if (log != nullptr)
		{
			log->WriteLine(ISolverLog::Jacobian);
		}
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

		auto& report = [&](
			const pair<Vector3D, Vector3D>& lhs, 
			const pair<Vector3D, Vector3D>& rhs)
		{
			double p, r;
			double e1, e2;
			
			p = Distance(lhs.first, rhs.first);
			e1 = residual.Set(p, w);

			auto nearRot = ExponentialMap::GetNearRotation(lhs.second, rhs.second);
			r = Distance(lhs.second, nearRot);
			e2 = residual.Set(r, w);

			if (log != nullptr && writeDebug)
			{
				log->Write(ISolverLog::Residual, L"%f\t%f\t", p, r);
			}

			error += e1 + e2;
		};

		report(last.body, d.body);
		report(last.foot[0], d.foot[0]);
		report(last.foot[1], d.foot[1]);

		if (log != nullptr && writeDebug)
		{
			log->WriteLine(
				ISolverLog::Console | ISolverLog::Debug,
				L"Residual Task = %.5f", error);
		}

		return error;
	}

	//------------------------------------------------------------------------------
	void LoadJacobian_DestinationTask(
		const ISolutionVector* s,
		const SolutionCoordinate& d,
		const Coefficient& w)
	{
		auto coordVar = SolutionCoordinate::VariableCount();

		double* jacobianDump = new double[12 * coordVar]{ 0 };
		int y = 0;

		const auto& last = s->GetLastPhase();

		unique_ptr<ISolutionVector> s2(s->Clone());
		const auto& last2 = s2->GetLastPhase();
		double step = 0.0001;

		int varPivot = (s->GetPhaseCount() - 2) * SolutionCoordinate::VariableCount();

		auto set = [&](double j)
		{
			jacobianDump[y * coordVar + varPivot] = j;
			jacobian.Set(varPivot++, w, j);
		};

		auto next = [&]()
		{
			y++;
			jacobian.NextFunction();
		};

		// body pos diff
		set(last.body.first.m[0] - d.body.first.m[0]);
		set(last.body.first.m[1] - d.body.first.m[1]);
		set(last.body.first.m[2] - d.body.first.m[2]);
		next();

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

			set(derivative);
		}
		next();

		// left foot pos diff
		set(last.foot[0].first.m[0] - d.foot[0].first.m[0]);
		set(last.foot[0].first.m[1] - d.foot[0].first.m[1]);
		set(last.foot[0].first.m[2] - d.foot[0].first.m[2]);
		next();

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

			set(derivative);
		}
		next();

		// right foot pos diff
		set(last.foot[1].first.m[0] - d.foot[1].first.m[0]);
		set(last.foot[1].first.m[1] - d.foot[1].first.m[1]);
		set(last.foot[1].first.m[2] - d.foot[1].first.m[2]);
		next();

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

			set(derivative);
		}
		next();

		if (log != nullptr)
		{
			for (int f = 0; f < y; ++f)
			{
				for (int v = 0; v < coordVar; ++v)
				{
					log->Write(ISolverLog::Jacobian, L"%e\t", jacobianDump[f * coordVar + v]);
				}
				log->WriteLine(ISolverLog::Jacobian);
			}
		}

		delete[] jacobianDump;
	}

	//------------------------------------------------------------------------------
	double LoadResidual_GeneralAcceleration(
		const ISolutionVector* s,
		const Coefficient& w,
		bool writeDebug)
	{
		double error = 0;

		auto& report = [&](double r)
		{
			auto e = residual.Set(r, w);

			if (log != nullptr && writeDebug)
			{
				log->Write(ISolverLog::Residual, L"%f\t", r);
			}

			error += e;
		};

		for (size_t i = 0; i < s->GetPhaseCount(); ++i)
		{
			//auto ga = s->GeneralAccelerationAt(s->GetPhaseTime(i), true);

			auto ga = GeneralAccelerationAt(s, i, false);

			report(Length(ga.body.first));
			report(Length(ga.body.second));

			report(Length(ga.leg[0].rot1));
			report(ga.leg[0].len1);
			report(Length(ga.leg[0].rot2));
			report(ga.leg[0].len2);
			report(Length(ga.leg[0].footRot));

			report(Length(ga.leg[1].rot1));
			report(ga.leg[1].len1);
			report(Length(ga.leg[1].rot2));
			report(ga.leg[1].len2);
			report(Length(ga.leg[1].footRot));
		}

		if (log != nullptr && writeDebug)
		{
			log->WriteLine(
				ISolverLog::Console | ISolverLog::Debug,
				L"Residual Accl = %.5f", error);
		}

		return error;
	}

	//------------------------------------------------------------------------------
	GeneralCoordinate GeneralAccelerationAt(const ISolutionVector* sv, int p, bool dump)
	{
		GeneralizedAccelerationCalculator calc(sv, p, dump);
		return calc.Get();
	}

	//------------------------------------------------------------------------------
	void LoadJacobian_GeneralAcceleration(
		const ISolutionVector* s,
		const Coefficient& w)
	{
		unique_ptr<ISolutionVector> s2(s->Clone());
		double step = 0.0001;

		auto next = [&] { jacobian.NextFunction(); };

		// 초기는 어쩔 수 없다고 치고
		for (int i = 0; i < 12; ++i)
		{
			next();
		}

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

				//bool toDump = v == 1;
				bool toDump = false;

				if (toDump)
				{
					s2->EnableIKDump(true, false);

					WindowsUtility::Debug(L"gc\n");
					s2->SetVariableAt(varOfs, reserved - step);
					GeneralCoordinate g0 = s2->GeneralCoordinateAt(s2->GetPhaseTime(p));

					s2->SetVariableAt(varOfs, reserved);
					GeneralCoordinate g1 = s2->GeneralCoordinateAt(s2->GetPhaseTime(p));

					s2->SetVariableAt(varOfs, reserved + step);
					GeneralCoordinate g2 = s2->GeneralCoordinateAt(s2->GetPhaseTime(p));

					s2->SetVariableAt(varOfs, reserved);

					g0.Dump_();
					g1.Dump_();
					g2.Dump_();

					s2->EnableIKDump(false, false);
				}

				s2->SetVariableAt(varOfs, reserved - step);
				auto gam = GeneralAccelerationAt(s2.get(), p, toDump);

				s2->SetVariableAt(varOfs, reserved);
				auto ga = GeneralAccelerationAt(s2.get(), p, toDump);

				s2->SetVariableAt(varOfs, reserved + step);
				auto gap = GeneralAccelerationAt(s2.get(), p, toDump);

				// 다시 원복
				s2->SetVariableAt(varOfs, reserved);

				if (toDump)
				{
					WindowsUtility::Debug(L"gc''\n");

					gam.Dump_();
					ga.Dump_();
					gap.Dump_();

					WindowsUtility::Debug(L"\n");
				}

				d[0 * coordVar + v] = SquaredLength((gap.body.first - gam.body.first) * ga.body.first) / (step * 2);
				d[1 * coordVar + v] = SquaredLength((gap.body.second - gam.body.second) * ga.body.second) / (step * 2);
				d[2 * coordVar + v] = SquaredLength((gap.leg[0].rot1 - gam.leg[0].rot1) * ga.leg[0].rot1) / (step * 2);
				d[3 * coordVar + v] = Square((gap.leg[0].len1 - gam.leg[0].len1) * ga.leg[0].len1) / (step * 2);
				d[4 * coordVar + v] = SquaredLength((gap.leg[0].rot2 - gam.leg[0].rot2) * ga.leg[0].rot2) / (step * 2);
				d[5 * coordVar + v] = Square((gap.leg[0].len2 - gam.leg[0].len2) * ga.leg[0].len2) / (step * 2);
				d[6 * coordVar + v] = SquaredLength((gap.leg[0].footRot - gam.leg[0].footRot) * ga.leg[0].footRot) / (step * 2);
				d[7 * coordVar + v] = SquaredLength((gap.leg[1].rot1 - gam.leg[1].rot1) * ga.leg[1].rot1) / (step * 2);
				d[8 * coordVar + v] = Square((gap.leg[1].len1 - gam.leg[1].len1) * ga.leg[1].len1) / (step * 2);
				d[9 * coordVar + v] = SquaredLength((gap.leg[1].rot2 - gam.leg[1].rot2) * ga.leg[1].rot2) / (step * 2);
				d[10 * coordVar + v] = Square((gap.leg[1].len2 - gam.leg[1].len2) * ga.leg[1].len2) / (step * 2);
				d[11 * coordVar + v] = SquaredLength((gap.leg[1].footRot - gam.leg[1].footRot) * ga.leg[1].footRot) / (step * 2);
			}

			if (log != nullptr)
			{ 
				for (int f = 0; f < 12; ++f)
				{
					for (int v = 0; v < coordVar; ++v)
					{
						log->Write(ISolverLog::Jacobian, L"%e\t", d[f * coordVar + v]);
					}
					log->WriteLine(ISolverLog::Jacobian);
				}
			}

			for (int f = 0; f < 12; ++f)
			{
				for (int v = 0; v < coordVar; ++v)
				{
					auto varOfs = (p - 1) * coordVar + v;

					jacobian.Set(varOfs, w, d[f * coordVar + v]);
				}
				next();
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