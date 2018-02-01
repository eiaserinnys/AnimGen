#pragma once

//------------------------------------------------------------------------------
// +
namespace VectorOperation
{
	//--------------------------------------------------------------------------
	// Assign
	struct Assign
	{
		template <typename V, typename Arg>
		__forceinline static void Evaluate(const int i, V& v, const Arg& arg)
		{
			v.m[i] = arg.Evaluate(i);
		}
	};

	//--------------------------------------------------------------------------
	// Unary
	struct Plus
	{
		template <typename Arg>
		__forceinline static const typename Arg::ValueType Evaluate(const int i, const Arg& arg)
		{
			return arg.Evaluate(i);
		}
	};

	struct Minus
	{
		template <typename Arg>
		__forceinline static const typename Arg::ValueType Evaluate(const int i, const Arg& arg)
		{
			return - arg.Evaluate(i);
		}
	};

	//--------------------------------------------------------------------------
	struct Add
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) + rhs.Evaluate(i);
		}
	};

	struct Subtract
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) - rhs.Evaluate(i);
		}
	};

	struct Multiply
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) * rhs.Evaluate(i);
		}
	};

	struct Divide
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) / rhs.Evaluate(i);
		}
	};

	struct Dot
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			typename Lhs::ValueType result = 0;
			for (int j = 0; j < Lhs::Dimension; ++j)
			{
				result += lhs.Evaluate(j) * rhs.Evaluate(j);
			}
			return result;
		}
	};

	struct Length
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return std::sqrt(Dot::Evaluate(i, lhs, rhs));
		}
	};

	struct Distance
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return std::sqrt(Dot::Evaluate(i, lhs - rhs, lhs - rhs));
		}
	};

	struct Cross2D
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return 
				lhs.Evaluate(0) * rhs.Evaluate(1) - 
				lhs.Evaluate(1) * rhs.Evaluate(0);
		}
	};

	struct Cross3D
	{
		template <typename Lhs, typename Rhs>
		__forceinline static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			const int d = Lhs::Dimension;
			return 
				lhs.Evaluate((i + 1) % d) * rhs.Evaluate((i + 2) % d) - 
				lhs.Evaluate((i + 2) % d) * rhs.Evaluate((i + 1) % d);
		}
	};
}

template <typename Arg>
const auto operator + (const Arg& arg)
{ return VectorUnaryExpression<Arg, VectorOperation::Plus>(arg); }

template <typename Arg>
const auto operator - (const Arg& arg)
{ return VectorUnaryExpression<Arg, VectorOperation::Minus>(arg); }

template <typename Lhs, typename Rhs>
const auto operator + (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Add>(lhs, rhs); }

template <typename Lhs, typename Rhs>
const auto operator - (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Subtract>(lhs, rhs); }

template <typename Lhs, typename Rhs>
const auto operator * (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Multiply>(lhs, rhs); }

template <typename Lhs, typename Rhs>
const auto operator / (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Divide>(lhs, rhs); }

template <
	typename Lhs, 
	typename Rhs,
	typename = std::enable_if_t<
		VectorArgument<Lhs>::Dimension == VectorArgument<Rhs>::Dimension, void>>
const auto Dot(const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Dot, 1>(lhs, rhs); }

template <typename Arg>
const auto Length(const Arg& arg)
{ return VectorBinaryExpression<Arg, Arg, VectorOperation::Length, 1>(arg, arg); }

template <
	typename Lhs, 
	typename Rhs,
	typename = std::enable_if_t<
		VectorArgument<Lhs>::Dimension == VectorArgument<Rhs>::Dimension, void>>
const auto Distance(const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Distance, 1>(lhs, rhs); }

template <
	typename Lhs, 
	typename Rhs,
	typename = std::enable_if_t<
		VectorArgument<Lhs>::Dimension == 2 && 
		VectorArgument<Rhs>::Dimension == 2, void>>
const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Cross2D, 1> Cross(const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Cross2D, 1>(lhs, rhs); }

template <
	typename Lhs, 
	typename Rhs,
	typename = std::enable_if_t<
		VectorArgument<Lhs>::Dimension == 3 && 
		VectorArgument<Rhs>::Dimension == 3, void>>
const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Cross3D> Cross(const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Cross3D>(lhs, rhs); }
