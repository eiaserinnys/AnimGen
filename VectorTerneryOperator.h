#pragma once

#include "VectorMacro.h"
#include "VectorExpressionArgument.h"

namespace Core
{
	namespace VectorOperator
	{
		template <typename L, typename M, typename R>
		struct TerneryOperator
		{
			typedef L LhsBaseType;
			typedef M MhsBaseType;
			typedef R RhsBaseType;

			typedef VectorExpressionArgument<L> LhsType;
			typedef VectorExpressionArgument<M> MhsType;
			typedef VectorExpressionArgument<R> RhsType;

			__forceinline void PreEvaluate(
				const LhsType& lhs, 
				const MhsType& mhs, 
				const RhsType& rhs) const
			{
				lhs.PreEvaluate();
				mhs.PreEvaluate();
				rhs.PreEvaluate();
			}
		};

		template <typename L, typename M, typename R>
		struct Lerp : public TerneryOperator<L, M, R>
		{
			typedef decltype(
				DECLVAL(LhsType) * DECLVAL(RhsType) + 
				DECLVAL(MhsType) * DECLVAL(RhsType)) ValueType;

			enum { Dimension = Max<LhsType::Dimension, MhsType::Dimension>::Value };

			typedef VectorT<ValueType, Dimension> VectorType;

			template <ENABLE_IF(
				LhsType::Dimension > 1 && 
				HAS_SAME_DIMENSION(LhsType, MhsType) && 
				IS_SCALAR(RhsType))>
			__forceinline const ValueType Evaluate(
				const int i, 
				const LhsType& lhs, 
				const MhsType& mhs, 
				const RhsType& rhs) const
			{
				return 
					lhs.Evaluate(i) * (1 - rhs.Evaluate(i)) +
					mhs.Evaluate(i) * rhs.Evaluate(i);
			}
		};
	}
}
