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
		__forceinline ValueType Evaluate() const
		{
			op.PreEvaluate(arg);
			return op.Evaluate(0, arg);
		}

		template <ENABLE_IF(Dimension == 1)>
		__forceinline operator ValueType() const
		{ return Evaluate(); }

		template <ENABLE_IF(Dimension > 1)>
		__forceinline VectorT<ValueType, Dimension> Evaluate() const
		{ return VectorT<ValueType, Dimension>(*this); }

		template <ENABLE_IF(Dimension > 1)>
		__forceinline operator VectorT<ValueType, Dimension>() const
		{ return Evaluate(); }

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

		VectorBinaryExpression(
			const LhsType& lhs, const RhsType& rhs) 
			: lhs(lhs), rhs(rhs)
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
		{ return Evaluate(); }

		template <ENABLE_IF(Dimension == 1)>
		__forceinline ValueType Evaluate() const
		{
			op.PreEvaluate(lhs, rhs);
			return op.Evaluate(0, lhs, rhs);
		}

		template <ENABLE_IF(Dimension > 1)>
		__forceinline operator VectorT<ValueType, Dimension>() const
		{ return Evaluate(); }

		template <ENABLE_IF(Dimension > 1)>
		__forceinline VectorT<ValueType, Dimension> Evaluate() const
		{ return VectorT<ValueType, Dimension>(*this); }

	private:
		const LhsType lhs;
		const RhsType rhs;
		Op op;
	};

	//------------------------------------------------------------------------------
	template <typename Op>
	class VectorTerneryExpression {
	public:
		typedef typename Op::LhsType LhsType;
		typedef typename Op::MhsType MhsType;
		typedef typename Op::RhsType RhsType;
		typedef typename Op::ValueType ValueType;
		enum { Dimension = Op::Dimension };

		VectorTerneryExpression(
			const LhsType& lhs, const MhsType& mhs, const RhsType& rhs) 
			: lhs(lhs), mhs(mhs), rhs(rhs)
		{
		}

		__forceinline void PreEvaluate() const
		{
			op.PreEvaluate(lhs, mhs, rhs);
		}

		__forceinline ValueType Evaluate(const int index) const
		{
			return op.Evaluate(index, lhs, mhs, rhs);
		}

		template <ENABLE_IF(Dimension == 1)>
		__forceinline ValueType Evalaute() const
		{
			op.PreEvaluate(lhs, mhs, rhs);
			return op.Evaluate(0, lhs, mhs, rhs);
		}

		template <ENABLE_IF(Dimension == 1)>
		__forceinline operator ValueType() const
		{ return Evalaute(); }

		template <ENABLE_IF(Dimension > 1)>
		__forceinline VectorT<ValueType, Dimension> Evaluate() const
		{ return VectorT<ValueType, Dimension>(*this); }

		template <ENABLE_IF(Dimension > 1)>
		__forceinline operator VectorT<ValueType, Dimension>() const
		{ return Evaluate(); }

	private:
		const LhsType lhs;
		const MhsType mhs;
		const RhsType rhs;
		Op op;
	};

	//------------------------------------------------------------------------------
	template <typename Op>
	struct VectorAssignment {
	public:
		typedef typename Op::LhsType VType;
		typedef typename Op::LhsType::ValueType VType_V;

		typedef typename Op::RhsType ExprType;
		typedef typename ExprType::ValueType ExprType_V;

		typedef VectorT<typename VType::ValueType, VType::Dimension> Vector;

		template <
			ENABLE_IF(
				IS_SAME_TYPE(typename Op::LhsBaseType, Vector) &&
				IS_CONVERTIBLE(VType_V, ExprType_V) &&
				(HAS_SAME_DIMENSION(VType, ExprType) || IS_SCALAR(ExprType)))>
		__forceinline void Evaluate(Vector& target, const ExprType& expr) const
		{
			expr.PreEvaluate();

			for (int i = 0; i < VType::Dimension; ++i)
			{
				target.m[i] = op.Evaluate(i, VType(target), expr);
			}
		}

		Op op;
	};

};