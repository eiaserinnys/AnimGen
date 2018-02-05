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

	public:
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

	public:
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

	public:
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

	public:
		double arg;
	};
}