#pragma once

//------------------------------------------------------------------------------
template <typename V, int D>
class VectorT : public VectorDescriptor<V, D> {
public:
	typedef V ValueType;
	typedef ValueType* Iterator;
	typedef const ValueType* ConstIterator;

	enum { Dimension = D };

	//--------------------------------------------------------------------------
	// constructor
	VectorT() = default;

	VectorT(const VectorT& rhs) { operator = (rhs); }

	template <typename = std::enable_if_t<Dimension == 2, ValueType>>
	VectorT(ValueType x, ValueType y)
	{ m[0] = x; m[1] = y; }

	template <typename = std::enable_if_t<Dimension == 3, ValueType>>
	VectorT(ValueType x, ValueType y, ValueType z)
	{ m[0] = x; m[1] = y; m[2] = z; }

	template <typename = std::enable_if_t<Dimension == 4, ValueType>>
	VectorT(ValueType x, ValueType y, ValueType z, ValueType w)
	{ m[0] = x; m[1] = y; m[2] = z; m[3] = w; }

	template <
		typename Arg,
		typename Op,
		typename = std::enable_if_t<
		std::is_same<
			ValueType,
			typename VectorArgument<Arg>::ValueType>::value &&
			(
				Dimension == VectorArgument<Arg>::Dimension ||
				VectorArgument<Arg>::Dimension == 1
			),
		void>>
		VectorT(const VectorUnaryExpression<Arg, Op>& expr)
	{
		VectorAssignment<
			VectorT,
			VectorUnaryExpression<Arg, Op>,
			VectorOperation::Assign>::Evaluate(*this, expr);
	}

	template <
		typename Lhs,
		typename Rhs,
		typename Op,
		typename = std::enable_if_t<
		std::is_same<
			ValueType,
			typename VectorArgument<Lhs>::ValueType>::value &&
			(
				Dimension == VectorArgument<Lhs>::Dimension ||
				VectorArgument<Lhs>::Dimension == 1
			),
		void>>
		VectorT(const VectorBinaryExpression<Lhs, Rhs, Op>& expr)
	{
		VectorAssignment<
			VectorT,
			VectorBinaryExpression<Lhs, Rhs, Op>,
			VectorOperation::Assign>::Evaluate(*this, expr);
	}

	VectorT& operator = (const VectorT& rhs)
	{
		ForEach([&](int i) { m[i] = rhs.m[i]; }); return *this;
	}

	//--------------------------------------------------------------------------
	// assignment
	template <
		typename Arg,
		typename Op,
		typename = std::enable_if_t<
		std::is_same<
			ValueType,
			typename VectorArgument<Arg>::ValueType>::value &&
			(
				Dimension == VectorArgument<Arg>::Dimension ||
				VectorArgument<Arg>::Dimension == 1
			),
		void>>
	VectorT& operator = (const VectorUnaryExpression<Arg, Op>& expr)
	{
		VectorAssignment<
			VectorT,
			VectorUnaryExpression<Arg, Op>,
			VectorOperation::Assign>::Evaluate(*this, expr);
		return *this;
	}

	template <
		typename Lhs,
		typename Rhs,
		typename Op,
		typename = std::enable_if_t<
			std::is_same<
			typename ValueType,
			typename VectorArgument<Lhs>::ValueType>::value &&
			(
				Dimension == VectorArgument<Lhs>::Dimension ||
				VectorArgument<Rhs>::Dimension == 1
			),
		void>>
	VectorT& operator = (const VectorBinaryExpression<Lhs, Rhs, Op>& expr)
	{
		VectorAssignment<
			VectorT,
			VectorBinaryExpression<Lhs, Rhs, Op>,
			VectorOperation::Assign>::Evaluate(*this, expr);
		return *this;
	}

	//--------------------------------------------------------------------------
	// etc
	VectorT& ForEach(const std::function<void(int)>& functor)
	{
		for (int i = 0; i < Dimension; ++i) { functor(i); }
		return *this;
	}

	VectorT& ForEach(const std::function<void(VectorT&, int)>& functor)
	{
		for (int i = 0; i < Dimension; ++i) { functor(*this, i); }
		return *this;
	}

	const ValueType Evaluate(int index) const { return m[index]; }
};
