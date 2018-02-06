#pragma once

#include "VectorMacro.h"
#include "VectorExpressionArgument.h"

namespace Core
{
	namespace VectorOperator
	{
		template <typename A>
		struct UnaryOperator
		{
			typedef A ArgBaseType;
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
}
