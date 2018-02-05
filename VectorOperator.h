#pragma once

#pragma once

#include "VectorMacro.h"

namespace Core
{
	//------------------------------------------------------------------------------
	// Unary
	namespace VectorOperator
	{
		template <typename A>
		struct UnaryOperator
		{
			typedef VectorExpressionArgument<A> ArgType;
			typedef typename ArgType::ValueType ValueType;
			enum { Dimension = ArgType::Dimension };
			typedef VectorT<ValueType, Dimension> VectorType;

			__forceinline void PreEvaluate(const ArgType& arg) const
			{
				arg.PreEvaluate();
			}
		};

		template <typename A>
		struct Plus : public UnaryOperator<A>
		{
			__forceinline const ValueType Evaluate(const int i, const ArgType& arg) const
			{
				return arg.Evaluate(i);
			}
		};

		template <typename A>
		struct Minus : public UnaryOperator<A>
		{
			__forceinline const ValueType Evaluate(const int i, const ArgType& arg) const
			{
				return -arg.Evaluate(i);
			}
		};

		template <typename A>
		struct Length : public UnaryOperator<A>
		{
			enum { Dimension = 1 };

			mutable ValueType evaluated;

			__forceinline void PreEvaluate(const ArgType& arg) const;

			__forceinline const ValueType Evaluate(const int i, const ArgType& arg) const
			{
				return evaluated;
			}
		};

		template <typename A>
		struct Normalize : public UnaryOperator<A>
		{
			mutable VectorType evaluated;

			__forceinline void PreEvaluate(const ArgType& arg) const;

			__forceinline const ValueType Evaluate(const int i, const ArgType& arg) const
			{
				return evaluated.Evaluate(i);
			}
		};
	}

	//------------------------------------------------------------------------------
	// Binary
	namespace VectorOperator
	{
		template <typename Lhs, typename Rhs>
		using ReturnType = typename MoreGenericType<
			typename Lhs::ValueType, typename Rhs::ValueType>::Type;

		struct BinaryOperator
		{
			template <typename Lhs, typename Rhs>
			struct IsValid
			{
				enum
				{
					Value = IS_CONVERTIBLE(
						typename Lhs::ValueType,
						typename Rhs::ValueType) &&
						(HAS_SAME_DIMENSION(Lhs, Rhs) ||
							IS_SCALAR(Lhs) ||
							IS_SCALAR(Rhs))
				};
			};

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
		struct Distance : public BinaryOperator
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
}
