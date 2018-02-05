#pragma once

#include "VectorMacro.h"

namespace Core
{

	//------------------------------------------------------------------------------
	// Unary
	namespace VectorOperation
	{
		struct UnaryOperator
		{
			template <typename Arg>
			__forceinline void PreEvaluate(const Arg& arg) const
			{
				arg.PreEvaluate();
			}
		};

		struct Plus : public UnaryOperator
		{
			template <typename Arg>
			__forceinline const typename Arg::ValueType Evaluate(const int i, const Arg& arg) const
			{
				return arg.Evaluate(i);
			}
		};

		struct Minus : public UnaryOperator
		{
			template <typename Arg>
			__forceinline const typename Arg::ValueType Evaluate(const int i, const Arg& arg) const
			{
				return -arg.Evaluate(i);
			}
		};

		template <typename V, int D>
		struct Length
		{
			mutable V evaluated;

			template <typename Arg>
			__forceinline void PreEvaluate(const Arg& arg) const;

			template <typename Arg>
			__forceinline const typename Arg::ValueType Evaluate(const int i, const Arg& arg) const
			{
				return evaluated;
			}
		};

		template <typename V, int D>
		struct Normalize
		{
			mutable VectorT<V, D> evaluated;

			template <typename Arg>
			__forceinline void PreEvaluate(const Arg& arg) const;

			template <typename Arg>
			__forceinline const typename Arg::ValueType Evaluate(const int i, const Arg& arg) const
			{
				return evaluated.Evaluate(i);
			}
		};
	}

	template <typename Arg>
	const auto operator + (const Arg& arg)
	{
		return VectorUnaryExpression<Arg, VectorOperation::Plus>(arg);
	}

	template <typename Arg>
	const auto operator - (const Arg& arg)
	{
		return VectorUnaryExpression<Arg, VectorOperation::Minus>(arg);
	}

	template <typename Arg>
	const auto Length(const Arg& arg)
	{
		return VectorUnaryExpression<
			Arg,
			VectorOperation::Length<typename Arg::ValueType, Arg::Dimension>, 1>(arg);
	}

	template <typename Arg>
	const auto Normalize(const Arg& arg)
	{
		return VectorUnaryExpression<
			Arg,
			VectorOperation::Normalize<typename Arg::ValueType, Arg::Dimension>>(arg);
	}

	//------------------------------------------------------------------------------
	// Binary
	namespace VectorOperation
	{
		template <typename Lhs, typename Rhs>
		using ReturnType = typename MoreGenericType<
			typename Lhs::ValueType, typename Rhs::ValueType>::Type;

		struct BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline void PreEvaluate(const Lhs& lhs, const Rhs& rhs) const
			{
				lhs.PreEvaluate();
				rhs.PreEvaluate();
			}
		};

		struct SelectLhs : public BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return lhs.Evaluate(i);
			}
		};

		struct SelectRhs : public BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return rhs.Evaluate(i);
			}
		};

		struct Add : public BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return lhs.Evaluate(i) + rhs.Evaluate(i);
			}
		};

		struct Subtract : public BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return lhs.Evaluate(i) - rhs.Evaluate(i);
			}
		};

		struct Multiply : public BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return lhs.Evaluate(i) * rhs.Evaluate(i);
			}
		};

		struct Divide : public BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return lhs.Evaluate(i) / rhs.Evaluate(i);
			}
		};

		template <typename V, int D>
		struct Dot
		{
			mutable V evaluated;

			template <typename Lhs, typename Rhs>
			__forceinline void PreEvaluate(const Lhs& lhs, const Rhs& rhs) const
			{
				VectorT<V, D> l, r;
				VectorAssignment<VectorT<V, D>, Lhs, SelectRhs>().Evaluate(l, lhs);
				VectorAssignment<VectorT<V, D>, Rhs, SelectRhs>().Evaluate(r, rhs);

				evaluated = 0;

				for (int j = 0; j < Lhs::Dimension; ++j)
				{
					evaluated += l.Evaluate(j) * r.Evaluate(j);
				}
			}

			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return evaluated;
			}
		};

		template <typename V, int D>
		struct Distance
		{
			mutable V evaluated;

			template <typename Lhs, typename Rhs>
			__forceinline void PreEvaluate(const Lhs& lhs, const Rhs& rhs) const;

			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return evaluated;
			}
		};

		struct Cross2D : public BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				return
					lhs.Evaluate(0) * rhs.Evaluate(1) -
					lhs.Evaluate(1) * rhs.Evaluate(0);
			}
		};

		template <typename V, int D>
		struct Cross3D : public BinaryOperator
		{
			mutable VectorT<V, D> l;
			mutable VectorT<V, D> r;

			template <typename Lhs, typename Rhs>
			__forceinline void PreEvaluate(const Lhs& lhs, const Rhs& rhs) const
			{
				VectorAssignment<VectorT<V, D>, Lhs, SelectRhs>().Evaluate(l, lhs);
				VectorAssignment<VectorT<V, D>, Rhs, SelectRhs>().Evaluate(r, rhs);
			}

			template <typename Lhs, typename Rhs>
			__forceinline const ReturnType<Lhs, Rhs> Evaluate(const int i, const Lhs& lhs, const Rhs& rhs) const
			{
				const int d = Lhs::Dimension;
				return
					l.Evaluate((i + 1) % d) * r.Evaluate((i + 2) % d) -
					l.Evaluate((i + 2) % d) * r.Evaluate((i + 1) % d);
			}
		};
	}

	template <typename Lhs, typename Rhs>
	const auto operator + (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Add>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs>
	const auto operator - (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Subtract>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs>
	const auto operator * (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Multiply>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs>
	const auto operator / (const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Divide>(lhs, rhs);
	}

	template <typename Lhs, typename Rhs,
		ENABLE_IF(HAS_SAME_DIMENSION(VectorExpressionArgument<Lhs>, VectorExpressionArgument<Rhs>))>
		const auto Dot(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<
			Lhs, Rhs,
			VectorOperation::Dot<typename Lhs::ValueType, Lhs::Dimension>, 1>(lhs, rhs);
	}

	template <
		typename Lhs, typename Rhs,
		ENABLE_IF(HAS_SAME_DIMENSION(VectorExpressionArgument<Lhs>, VectorExpressionArgument<Rhs>))>
		const auto Distance(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<
			Lhs, Rhs,
			VectorOperation::Distance<typename Lhs::ValueType, Lhs::Dimension>, 1>(lhs, rhs);
	}

	template <
		typename Lhs, typename Rhs,
		ENABLE_IF(DIM(VectorExpressionArgument<Lhs>) == 2 && DIM(VectorExpressionArgument<Rhs>) == 2)>
		const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Cross2D, 1> Cross(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Cross2D, 1>(lhs, rhs);
	}

	template <
		typename Lhs, typename Rhs,
		ENABLE_IF(DIM(VectorExpressionArgument<Lhs>) == 3 && DIM(VectorExpressionArgument<Rhs>) == 3)>
		__forceinline auto Cross(const Lhs& lhs, const Rhs& rhs)
	{
		return VectorBinaryExpression<
			Lhs, Rhs,
			VectorOperation::Cross3D<typename Lhs::ValueType, Lhs::Dimension> >(lhs, rhs);
	}

	//------------------------------------------------------------------------------
	template <typename V, int D>
	template <typename Arg>
	__forceinline void VectorOperation::Length<V, D>::PreEvaluate(const Arg& arg) const
	{
		VectorT<V, D> v;
		VectorAssignment<VectorT<V, D>, Arg, SelectRhs>().Evaluate(v, arg);
		evaluated = std::sqrt(::Dot(arg, arg));
	}

	template <typename V, int D>
	template <typename Arg>
	__forceinline void VectorOperation::Normalize<V, D>::PreEvaluate(const Arg& arg) const
	{
		VectorAssignment<VectorT<V, D>, Arg, SelectRhs>().Evaluate(evaluated, arg);
		V length = ::Length(evaluated);
		if (length > 0) { evaluated = evaluated / length; }
	}

	template <typename V, int D>
	template <typename Lhs, typename Rhs>
	__forceinline void VectorOperation::Distance<V, D>::PreEvaluate(const Lhs& lhs, const Rhs& rhs) const
	{
		VectorT<V, D> l, r;
		VectorAssignment<VectorT<V, D>, Lhs, SelectRhs>().Evaluate(l, lhs);
		VectorAssignment<VectorT<V, D>, Rhs, SelectRhs>().Evaluate(r, rhs);
		evaluated = ::Length(l - r);
	}

};