#pragma once

#include <type_traits>
#include <functional>

//------------------------------------------------------------------------------
template <typename V>
class VectorArgument {
public:
	typedef typename V::ValueType ValueType;
	enum { Dimension = V::Dimension };

	VectorArgument(const V& arg) : arg(arg) {}

	typename const V::ValueType Evaluate(const int index) const
	{ return arg.Evaluate(index); }

private:
	const V& arg;
};

//------------------------------------------------------------------------------
template <>
class VectorArgument<float> {
public:
	typedef float ValueType;
	enum { Dimension = 1 };

	VectorArgument(const float arg) : arg(arg) {}

	typename const float Evaluate(const int index) const { return arg; }

private:
	float arg;
};

//------------------------------------------------------------------------------
template <>
class VectorArgument<double> {
public:
	typedef double ValueType;
	enum { Dimension = 1 };

	VectorArgument(const double arg) : arg(arg) {}

	typename const double Evaluate(const int index) const { return arg; }

private:
	double arg;
};

//------------------------------------------------------------------------------
template <typename Arg, typename Op>
class VectorUnaryExpression {
public:
	typedef VectorArgument<Arg> ArgType;

	typedef typename ArgType::ValueType ValueType;

	enum { Dimension = ArgType::Dimension };

	VectorUnaryExpression(const Arg& arg) : arg(arg) {}

	typename const ArgType::ValueType Evaluate(const int index) const
	{ return Op::Evaluate(index, arg); }

private:
	const ArgType arg;
};

//------------------------------------------------------------------------------
template <typename Lhs, typename Rhs, typename Op, int D = 0>
class VectorBinaryExpression {
public:
	typedef VectorArgument<Lhs> LhsType;
	typedef VectorArgument<Rhs> RhsType;

	typedef typename LhsType::ValueType ValueType;

	enum 
	{ 
		Dimension = 
			D == 0 ? 
				(LhsType::Dimension > RhsType::Dimension ? 
					LhsType::Dimension : RhsType::Dimension) :
				D
	};

	template <typename = std::enable_if_t<
		std::is_same<
			typename LhsType::ValueType,
			typename RhsType::ValueType>::value &&
			(
				LhsType::Dimension == RhsType::Dimension ||
				LhsType::Dimension == 1 ||
				RhsType::Dimension == 1
			),
		void>>
	VectorBinaryExpression(const Lhs& lhs, const Rhs& rhs) : lhs(lhs), rhs(rhs) {}

	typename const LhsType::ValueType Evaluate(const int index) const
	{ return Op::Evaluate(index, lhs, rhs); }

	template <typename = std::enable_if_t<Dimension == 1, void>>
	operator ValueType() const
	{ return Op::Evaluate(0, lhs, rhs); }

private:
	const LhsType lhs;
	const RhsType rhs;
};

//------------------------------------------------------------------------------
template <typename V, typename Expr, typename Op>
struct VectorAssignment {
public:
	template <typename = std::enable_if_t<
		std::is_same<
			typename V::ValueType,
			typename Expr::ValueType>::value &&
		(
			V::Dimension == Expr::Dimension ||
			Expr::Dimension == 1
		),
		void>>
	static void Evaluate(V& target, const Expr& expr)
	{
		for (int i = 0; i < V::Dimension; ++i)
		{
			Op::Evaluate(i, target, expr);
		}
	}
};