#pragma once

#include "VectorMacro.h"
#include "VectorOperator.h"

namespace Core
{
	//------------------------------------------------------------------------------
	// Unary
	template <typename A>
	decltype(auto) operator + (const A& arg)
	{
		return VectorUnaryExpression<VectorOperator::Plus<A>>(arg);
	}

	template <typename A>
	decltype(auto) operator - (const A& arg)
	{
		return VectorUnaryExpression<VectorOperator::Minus<A>>(arg);
	}

	template <typename A>
	decltype(auto) Length(const A& arg)
	{
		return VectorUnaryExpression<VectorOperator::Length<A>>(arg);
	}

	template <typename A>
	decltype(auto) SquaredLength(const A& arg)
	{
		return VectorUnaryExpression<VectorOperator::SquaredLength<A>>(arg);
	}

	template <typename A>
	decltype(auto) Normalize(const A& arg)
	{
		return VectorUnaryExpression<VectorOperator::Normalize<A>>(arg);
	}

	//------------------------------------------------------------------------------
	// Binary
	template <typename L, typename R>
	decltype(auto) operator + (const L& lhs, const R& rhs)
	{
		return VectorBinaryExpression<VectorOperator::Add<L, R>>(lhs, rhs);
	}

	template <typename L, typename R>
	decltype(auto) operator - (const L& lhs, const R& rhs)
	{
		return VectorBinaryExpression<VectorOperator::Subtract<L, R>>(lhs, rhs);
	}

	template <typename L, typename R>
	decltype(auto) operator * (const L& lhs, const R& rhs)
	{
		return VectorBinaryExpression<VectorOperator::Multiply<L, R>>(lhs, rhs);
	}

	template <typename L, typename R>
	decltype(auto) operator / (const L& lhs, const R& rhs)
	{
		return VectorBinaryExpression<VectorOperator::Divide<L, R>>(lhs, rhs);
	}

	template <typename L, typename R>
	decltype(auto) Dot(const L& lhs, const R& rhs)
	{
		return VectorBinaryExpression<VectorOperator::Dot<L, R>>(lhs, rhs);
	}

	template <typename L, typename R>
	decltype(auto) Distance(const L& lhs, const R& rhs)
	{
		return VectorBinaryExpression<VectorOperator::Distance<L, R>>(lhs, rhs);
	}

	template <typename L, typename R>
	decltype(auto) SquaredDistance(const L& lhs, const R& rhs)
	{
		return VectorBinaryExpression<VectorOperator::SquaredDistance<L, R>>(lhs, rhs);
	}

	template <
		typename L, typename R,
		ENABLE_IF(
			DIM(VectorExpressionArgument<L>) == 2 && 
			DIM(VectorExpressionArgument<R>) == 2)>
	__forceinline auto Cross(const L& lhs, const R& rhs) -> VectorBinaryExpression<VectorOperator::Cross2D<L, R>>
	{
		return VectorBinaryExpression<VectorOperator::Cross2D<L, R>>(lhs, rhs);
	}

	template <
		typename L, typename R,
		ENABLE_IF(
			DIM(VectorExpressionArgument<L>) == 3 && 
			DIM(VectorExpressionArgument<R>) == 3)>
	__forceinline auto Cross(const L& lhs, const R& rhs) -> VectorBinaryExpression<VectorOperator::Cross3D<L, R>>
	{
		return VectorBinaryExpression<VectorOperator::Cross3D<L, R>>(lhs, rhs);
	}

	//------------------------------------------------------------------------------
	// Ternery
	template <typename L, typename M, typename R>
	__forceinline auto Lerp(const L& lhs, const M& mhs, const R& rhs)
	{
		return VectorTerneryExpression<VectorOperator::Lerp<L, M, R>>(lhs, mhs, rhs);
	}

	//------------------------------------------------------------------------------
	template <typename A>
	__forceinline void VectorOperator::Length<A>::PreEvaluate(const ArgType& arg) const
	{
		VectorType v;
		VectorAssignment<SelectRhs<VectorType, A>>().Evaluate(v, arg);
		evaluated = std::sqrt(Core::Dot(arg, arg));
	}

	template <typename A>
	__forceinline void VectorOperator::SquaredLength<A>::PreEvaluate(const ArgType& arg) const
	{
		VectorType v;
		VectorAssignment<SelectRhs<VectorType, A>>().Evaluate(v, arg);
		evaluated = Core::Dot(arg, arg);
	}

	template <typename A>
	__forceinline void VectorOperator::Normalize<A>::PreEvaluate(const ArgType& arg) const
	{
		VectorAssignment<SelectRhs<VectorType, A>>().Evaluate(evaluated, arg);
		ValueType length = Core::Length(evaluated);
		if (length > 0) { evaluated = evaluated / length; }
	}

	template <typename L, typename R>
	__forceinline void VectorOperator::Distance<L, R>::PreEvaluate(const LhsType& lhs, const RhsType& rhs) const
	{
		VectorType l, r;
		VectorAssignment<SelectRhs<VectorType, L>>().Evaluate(l, lhs);
		VectorAssignment<SelectRhs<VectorType, R>>().Evaluate(r, rhs);
		evaluated = Core::Length(l - r);
	}

	template <typename L, typename R>
	__forceinline void VectorOperator::SquaredDistance<L, R>::PreEvaluate(const LhsType& lhs, const RhsType& rhs) const
	{
		VectorType l, r;
		VectorAssignment<SelectRhs<VectorType, L>>().Evaluate(l, lhs);
		VectorAssignment<SelectRhs<VectorType, R>>().Evaluate(r, rhs);
		evaluated = Core::SquaredLength(l - r);
	}

}; 