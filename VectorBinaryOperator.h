#pragma once

#include "VectorMacro.h"
#include "VectorExpressionArgument.h"

namespace Core
{
	namespace VectorOperator
	{
		template <typename L, typename R>
		struct BinaryOperator
		{
			typedef L LhsBaseType;
			typedef R RhsBaseType;

			typedef VectorExpressionArgument<L> LhsType;
			typedef VectorExpressionArgument<R> RhsType;

			typedef typename MoreGenericType<
				typename LhsType::ValueType,
				typename RhsType::ValueType>::Type ValueType;

			enum { Dimension = Max<LhsType::Dimension, RhsType::Dimension>::Value };

			typedef VectorT<ValueType, Dimension> VectorType;

			template<ENABLE_IF(
				IS_CONVERTIBLE(
					typename LhsType::ValueType,
					typename RhsType::ValueType) &&
					(HAS_SAME_DIMENSION(LhsType, RhsType) ||
						IS_SCALAR(LhsType) ||
						IS_SCALAR(RhsType)))>
				BinaryOperator()
			{
			}

			__forceinline void PreEvaluate(const LhsType& lhs, const RhsType& rhs) const
			{
				lhs.PreEvaluate();
				rhs.PreEvaluate();
			}
		};

		template <typename L, typename R>
		struct SelectLhs : public BinaryOperator<L, R>
		{
			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return lhs.Evaluate(i);
			}
		};

		template <typename L, typename R>
		struct SelectRhs : public BinaryOperator<L, R>
		{
			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return rhs.Evaluate(i);
			}
		};

		template <typename L, typename R>
		struct Add : public BinaryOperator<L, R>
		{
			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return lhs.Evaluate(i) + rhs.Evaluate(i);
			}
		};

		template <typename L, typename R>
		struct Subtract : public BinaryOperator<L, R>
		{
			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return lhs.Evaluate(i) - rhs.Evaluate(i);
			}
		};

		template <typename L, typename R>
		struct Multiply : public BinaryOperator<L, R>
		{
			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return lhs.Evaluate(i) * rhs.Evaluate(i);
			}
		};

		template <typename L, typename R>
		struct Divide : public BinaryOperator<L, R>
		{
			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return lhs.Evaluate(i) / rhs.Evaluate(i);
			}
		};

		template <typename L, typename R>
		struct Dot : public BinaryOperator<L, R>
		{
			enum { Dimension = 1 };

			mutable ValueType evaluated = 0;

			template <ENABLE_IF(HAS_SAME_DIMENSION(LhsType, RhsType))>
			Dot() {}

			__forceinline void PreEvaluate(const LhsType& lhs, const RhsType& rhs) const
			{
				VectorType l, r;
				VectorAssignment<SelectRhs<VectorType, LhsType>>().Evaluate(l, lhs);
				VectorAssignment<SelectRhs<VectorType, RhsType>>().Evaluate(r, rhs);

				for (int j = 0; j < LhsType::Dimension; ++j)
				{
					evaluated += l.Evaluate(j) * r.Evaluate(j);
				}
			}

			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return evaluated;
			}
		};

		template <typename L, typename R>
		struct Distance : public BinaryOperator<L, R>
		{
			enum { Dimension = 1 };

			mutable ValueType evaluated;

			template <ENABLE_IF(HAS_SAME_DIMENSION(LhsType, RhsType))>
			Distance() {}

			__forceinline void PreEvaluate(const LhsType& lhs, const RhsType& rhs) const;

			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return evaluated;
			}
		};

		template <typename L, typename R>
		struct Cross2D : public BinaryOperator<L, R>
		{
			enum { Dimension = 1 };

			template <ENABLE_IF(DIM(LhsType) == 2 && DIM(RhsType) == 2)>
			Cross2D()
			{
			}

			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				return
					lhs.Evaluate(0) * rhs.Evaluate(1) -
					lhs.Evaluate(1) * rhs.Evaluate(0);
			}
		};

		template <typename L, typename R>
		struct Cross3D : public BinaryOperator<L, R>
		{
			mutable VectorType l, r;

			template <ENABLE_IF(DIM(LhsType) == 3 && DIM(RhsType) == 3)>
			Cross3D()
			{}

			__forceinline void PreEvaluate(const LhsType& lhs, const RhsType& rhs) const
			{
				VectorAssignment<SelectRhs<VectorType, L>>().Evaluate(l, lhs);
				VectorAssignment<SelectRhs<VectorType, R>>().Evaluate(r, rhs);
			}

			__forceinline const ValueType Evaluate(const int i, const LhsType& lhs, const RhsType& rhs) const
			{
				const int d = Dimension;
				return
					l.Evaluate((i + 1) % d) * r.Evaluate((i + 2) % d) -
					l.Evaluate((i + 2) % d) * r.Evaluate((i + 1) % d);
			}
		};
	}
}
