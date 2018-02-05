#pragma once

#include "VectorMacro.h"

namespace Core
{

	template <typename V, int D>
	class VectorT;

	//------------------------------------------------------------------------------
	template <typename V>
	class VectorExpressionArgument {
	public:
		typedef typename V::ValueType ValueType;
		enum { Dimension = V::Dimension };

		VectorExpressionArgument(const V& arg) : arg(arg) {}

		__forceinline void PreEvaluate() const
		{
			arg.PreEvaluate();
		}

		__forceinline typename const V::ValueType Evaluate(const int index) const
		{
			return arg.Evaluate(index);
		}

	private:
		const V& arg;
	};

	//------------------------------------------------------------------------------
	template <>
	class VectorExpressionArgument<int> {
	public:
		typedef int ValueType;
		enum { Dimension = 1 };

		VectorExpressionArgument(const int arg) : arg(arg) {}

		__forceinline void PreEvaluate() const
		{
		}

		__forceinline typename const int Evaluate(const int index) const
		{
			return arg;
		}

	private:
		int arg;
	};

	//------------------------------------------------------------------------------
	template <>
	class VectorExpressionArgument<float> {
	public:
		typedef float ValueType;
		enum { Dimension = 1 };

		VectorExpressionArgument(const float arg) : arg(arg) {}

		__forceinline void PreEvaluate() const
		{
		}

		__forceinline typename const float Evaluate(const int index) const
		{
			return arg;
		}

	private:
		float arg;
	};

	//------------------------------------------------------------------------------
	template <>
	class VectorExpressionArgument<double> {
	public:
		typedef double ValueType;
		enum { Dimension = 1 };

		VectorExpressionArgument(const double arg) : arg(arg)
		{
		}

		__forceinline void PreEvaluate() const
		{
		}

		__forceinline typename const double Evaluate(const int index) const
		{
			return arg;
		}

	private:
		double arg;
	};

	//------------------------------------------------------------------------------
	template <typename Arg, typename Op, int D = 0>
	class VectorUnaryExpression {
	public:
		typedef VectorExpressionArgument<Arg> ArgType;

		typedef typename ArgType::ValueType ValueType;

		enum { Dimension = D == 0 ? ArgType::Dimension : D };

		VectorUnaryExpression(const Arg& arg) : arg(arg) {}

		__forceinline void PreEvaluate() const
		{
			op.PreEvaluate(arg);
		}

		__forceinline typename const ArgType::ValueType Evaluate(const int index) const
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
	template <typename Lhs, typename Rhs, typename Op, int D = 0>
	class VectorBinaryExpression {
	public:
		typedef VectorExpressionArgument<Lhs> LhsType;
		typedef VectorExpressionArgument<Rhs> RhsType;

		typedef typename MoreGenericType<
			typename LhsType::ValueType,
			typename RhsType::ValueType>::Type ValueType;

		enum
		{
			Dimension =
			D == 0 ?
			(LhsType::Dimension > RhsType::Dimension ?
				LhsType::Dimension : RhsType::Dimension) :
			D
		};

		template <
			ENABLE_IF(
				IS_CONVERTIBLE(typename LhsType::ValueType, typename RhsType::ValueType) &&
				(HAS_SAME_DIMENSION(LhsType, RhsType) ||
					IS_SCALAR(LhsType) ||
					IS_SCALAR(RhsType))
			)>
			VectorBinaryExpression(const Lhs& lhs, const Rhs& rhs) : lhs(lhs), rhs(rhs)
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
	template <typename Lhs, typename Mhs, typename Rhs, typename Op, int D = 0>
	class VectorTernaryExpression {
	public:
		typedef VectorExpressionArgument<Lhs> LhsType;
		typedef VectorExpressionArgument<Mhs> MhsType;
		typedef VectorExpressionArgument<Rhs> RhsType;

		typedef typename MoreGenericType<
			typename MoreGenericType<
				typename LhsType::ValueType,
				typename MhsType::ValueType>::Type, 
			typename RhsType::ValueType>::Type ValueType;

		enum
		{
			Dimension =
			D == 0 ?
			std::max(
				std::max(Lhs::Dimension, Mhs::Dimension),
				Rhs::Dimension) :
			D
		};

		template <
			ENABLE_IF(
				IS_CONVERTIBLE(
					typename LhsType::ValueType, 
					typename RhsType::ValueType) &&
				(HAS_SAME_DIMENSION(LhsType, RhsType) ||
					IS_SCALAR(LhsType) ||
					IS_SCALAR(RhsType))
			)>
			VectorTernaryExpression(const Lhs& lhs, const Rhs& rhs) : lhs(lhs), rhs(rhs)
		{
		}

		__forceinline void PreEvaluate() const
		{
			op.PreEvaluate(lhs, mhs, rhs);
		}

		__forceinline typename ValueType Evaluate(const int index) const
		{
			return op.Evaluate(index, lhs, rhs);
		}

		template <ENABLE_IF(Dimension == 1)>
		__forceinline operator ValueType() const
		{
			op.PreEvaluate(lhs, mhs, rhs);
			return op.Evaluate(0, lhs, msh, rhs);
		}

		template <ENABLE_IF(Dimension > 1)>
		__forceinline operator VectorT<ValueType, Dimension>() const
		{
			return VectorT<ValueType, Dimension>(*this);
		}

	private:
		const LhsType lhs;
		const MhsType mhs;
		const RhsType rhs;
		Op op;
	};

	//------------------------------------------------------------------------------
	template <typename V, typename Expr, typename Op>
	struct VectorAssignment {
	public:
		template <
			ENABLE_IF(
				IS_CONVERTIBLE(typename V::ValueType, typename Expr::ValueType) &&
				(HAS_SAME_DIMENSION(V, Expr) || IS_SCALAR(Expr))
			)>
			__forceinline void Evaluate(V& target, const Expr& expr) const
		{
			expr.PreEvaluate();

			for (int i = 0; i < V::Dimension; ++i)
			{
				target.m[i] = op.Evaluate(i, target, expr);
			}
		}

		Op op;
	};

};