#pragma once

//------------------------------------------------------------------------------
// +
namespace VectorOperation
{
	struct Assign
	{
		template <typename V, typename Arg>
		static void Evaluate(const int i, V& v, const Arg& arg)
		{
			v.m[i] = arg.Evaluate(i);
		}
	};

	struct Plus
	{
		template <typename Arg>
		static const typename Arg::ValueType Evaluate(const int i, const Arg& arg)
		{
			return arg.Evaluate(i);
		}
	};

	struct Minus
	{
		template <typename Arg>
		static const typename Arg::ValueType Evaluate(const int i, const Arg& arg)
		{
			return - arg.Evaluate(i);
		}
	};

	struct Add
	{
		template <typename Lhs, typename Rhs>
		static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) + rhs.Evaluate(i);
		}
	};

	struct Subtract
	{
		template <typename Lhs, typename Rhs>
		static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) - rhs.Evaluate(i);
		}
	};

	struct Multiply
	{
		template <typename Lhs, typename Rhs>
		static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) * rhs.Evaluate(i);
		}
	};

	struct Divide
	{
		template <typename Lhs, typename Rhs>
		static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return lhs.Evaluate(i) / rhs.Evaluate(i);
		}
	};

	struct Dot
	{
		template <typename Lhs, typename Rhs>
		static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			typename Lhs::ValueType result = 0;
			for (int j = 0; j < Lhs::Dimension; ++j)
			{
				result += lhs.Evaluate(j) * rhs.Evaluate(j);
			}
			return result;
		}
	};

	struct Cross2D
	{
		template <typename Lhs, typename Rhs>
			static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			return 
				lhs.Evaluate(0) * rhs.Evaluate(1) - 
				lhs.Evaluate(1) * rhs.Evaluate(0);
		}
	};

	struct Cross3D
	{
		template <typename Lhs, typename Rhs>
		static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
		{
			const int d = Lhs::Dimension;
			return 
				lhs.Evaluate((i + 1) % d) * rhs.Evaluate((i + 2) % d) - 
				lhs.Evaluate((i + 2) % d) * rhs.Evaluate((i + 1) % d);
		}
	};
}

template <typename Arg>
const VectorUnaryExpression<Arg, VectorOperation::Plus> operator + (const Arg& arg)
{ return VectorUnaryExpression<Arg, VectorOperation::Plus>(arg); }

template <typename Arg>
const VectorUnaryExpression<Arg, VectorOperation::Minus> operator - (const Arg& arg)
{ return VectorUnaryExpression<Arg, VectorOperation::Minus>(arg); }

template <typename Lhs, typename Rhs>
const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Add> operator + (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Add>(lhs, rhs); }

template <typename Lhs, typename Rhs>
const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Subtract> operator - (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Subtract>(lhs, rhs); }

template <typename Lhs, typename Rhs>
const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Multiply> operator * (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Multiply>(lhs, rhs); }

template <typename Lhs, typename Rhs>
const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Divide> operator / (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Divide>(lhs, rhs); }

template <
	typename Lhs, 
	typename Rhs,
	typename = std::enable_if_t<
		VectorArgument<Lhs>::Dimension == VectorArgument<Rhs>::Dimension, void>>
const VectorBinaryExpression<Lhs, Rhs, VectorOperation::Dot, 1> Dot(const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, VectorOperation::Dot, 1>(lhs, rhs); }

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
