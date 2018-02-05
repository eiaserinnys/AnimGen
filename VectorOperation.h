#pragma once

#include "VectorMacro.h"

namespace Core
{
	//------------------------------------------------------------------------------
	// Unary
	template <typename Arg>
	const auto operator + (const Arg& arg)
	{
		return VectorUnaryExpression<VectorOperator::Plus<Arg>>(arg);
	}

	template <typename Arg>
	const auto operator - (const Arg& arg)
	{
		return VectorUnaryExpression<VectorOperator::Minus<Arg>>(arg);
	}

	template <typename Arg>
	const auto Length(const Arg& arg)
	{
		return VectorUnaryExpression<VectorOperator::Length<Arg>>(arg);
	}

	template <typename Arg>
	const auto Normalize(const Arg& arg)
	{
		return VectorUnaryExpression<VectorOperator::Normalize<Arg>>(arg);
	}

	//------------------------------------------------------------------------------
	// Binary
	template <typename Lhs, typename Rhs>
	const auto operator + (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperator::Add>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs>
	const auto operator - (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperator::Subtract>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs>
	const auto operator * (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperator::Multiply>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs>
	const auto operator / (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperator::Divide>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs,
		ENABLE_IF(HAS_SAME_DIMENSION(VectorExpressionArgument<Lhs>, VectorExpressionArgument<Rhs>))>
		const auto Dot(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<
			Lhs, Rhs,
			VectorOperator::Dot<typename Lhs::ValueType, Lhs::Dimension>, 1>(lhs, rhs);
	}

	template <
		typename Lhs, typename Rhs,
		ENABLE_IF(HAS_SAME_DIMENSION(VectorExpressionArgument<Lhs>, VectorExpressionArgument<Rhs>))>
		const auto Distance(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<
			Lhs, Rhs,
			VectorOperator::Distance<typename Lhs::ValueType, Lhs::Dimension>, 1>(lhs, rhs);
	}

	template <
		typename Lhs, typename Rhs,
		ENABLE_IF(DIM(VectorExpressionArgument<Lhs>) == 2 && DIM(VectorExpressionArgument<Rhs>) == 2)>
		const VectorBinaryExpression<Lhs, Rhs, VectorOperator::Cross2D, 1> Cross(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperator::Cross2D, 1>(lhs, rhs);
	}

	template <
		typename Lhs, typename Rhs,
		ENABLE_IF(DIM(VectorExpressionArgument<Lhs>) == 3 && DIM(VectorExpressionArgument<Rhs>) == 3)>
		__forceinline auto Cross(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<
			Lhs, Rhs,
			VectorOperator::Cross3D<typename Lhs::ValueType, Lhs::Dimension> >(lhs, rhs);
	}

	//------------------------------------------------------------------------------
	template <typename A>
	__forceinline void VectorOperator::Length<A>::PreEvaluate(const ArgType& arg) const
	{
		VectorType v;
		VectorAssignment<VectorType, A, SelectRhs>().Evaluate(v, arg.arg);
		evaluated = std::sqrt(::Dot(arg, arg));
	}

	template <typename A>
	__forceinline void VectorOperator::Normalize<A>::PreEvaluate(const ArgType& arg) const
	{
		VectorAssignment<VectorType, A, SelectRhs>().Evaluate(evaluated, arg.arg);
		ValueType length = ::Length(evaluated);
		if (length > 0) { evaluated = evaluated / length; }
	}

	template <typename V, int D>
	template <typename Lhs, typename Rhs>
	__forceinline void VectorOperator::Distance<V, D>::PreEvaluate(const Lhs& lhs, const Rhs& rhs) const
	{
		VectorT<V, D> l, r;
		VectorAssignment<VectorT<V, D>, Lhs, SelectRhs>().Evaluate(l, lhs);
		VectorAssignment<VectorT<V, D>, Rhs, SelectRhs>().Evaluate(r, rhs);
		evaluated = ::Length(l - r);
	}

}; 