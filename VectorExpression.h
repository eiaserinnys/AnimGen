#pragma once

#include "VectorMacro.h"
#include "VectorExpressionArgument.h"
#include "VectorOperator.h"

namespace Core
{

	template <typename V, int D>
	class VectorT;

	//------------------------------------------------------------------------------
	template <typename Op>
	class VectorUnaryExpression {
	public:
		typedef typename Op::ArgType ArgType;
		typedef typename Op::ValueType ValueType;
		enum { Dimension = Op::Dimension };

		VectorUnaryExpression(const ArgType& arg) : arg(arg) {}

		__forceinline void PreEvaluate() const
		{
			op.PreEvaluate(arg);
		}

		__forceinline typename const ValueType Evaluate(const int index) const
		{
			return op.Evaluate(index, arg);
		}

		template <ENABLE_IF(Dimension == 1)>
		__forceinline operator ValueType() const
		{
			op.PreEvaluate(arg);
			return op.Evaluate(0, arg);
		}

		template <ENABLE_IF(Dimension > 1)>
		__forceinline operator VectorT<ValueType, Dimension>() const
		{
			return VectorT<ValueType, Dimension>(*this);
		}

	private:
		const ArgType arg;
		Op op;
	};

	//------------------------------------------------------------------------------
	template <typename Op>
	class VectorBinaryExpression {
	public:
		typedef typename Op::LhsType LhsType;
		typedef typename Op::RhsType RhsType;
		typedef typename Op::ValueType ValueType;
		enum { Dimension = Op::Dimension };

		VectorBinaryExpression(const LhsType& lhs, const RhsType& rhs) : lhs(lhs), rhs(rhs)
		{
		}

		__forceinline void PreEvaluate() const
		{
			op.PreEvaluate(lhs, rhs);
		}

		__forceinline ValueType Evaluate(const int index) const
		{
			return op.Evaluate(index, lhs, rhs);
		}

		template <ENABLE_IF(Dimension == 1)>
		__forceinline operator ValueType() const
		{
			op.PreEvaluate(lhs, rhs);
			return op.Evaluate(0, lhs, rhs);
		}

		template <ENABLE_IF(Dimension > 1)>
		__forceinline operator VectorT<ValueType, Dimension>() const
		{
			return VectorT<ValueType, Dimension>(*this);
		}

	private:
		const LhsType lhs;
		const RhsType rhs;
		Op op;
	};

	//------------------------------------------------------------------------------
	template <typename V, typename Expr, typename Op>
	struct VectorAssignment {
	public:
		typedef VectorExpressionArgument<V> VType;
		typedef VectorExpressionArgument<Expr> ExprType;

		template <
			ENABLE_IF(
				IS_CONVERTIBLE(typename V::ValueType, typename Expr::ValueType) &&
				(HAS_SAME_DIMENSION(V, Expr) || IS_SCALAR(Expr)))>
		__forceinline void Evaluate(V& target, const ExprType& expr) const
		{
			expr.PreEvaluate();

			for (int i = 0; i < V::Dimension; ++i)
			{
				target.m[i] = op.Evaluate(i, VType(target), expr);
			}
		}

		Op op;
	};

};