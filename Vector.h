#pragma once

#include <type_traits>
#include <functional>

#include "VectorDescriptor.h"
#include "VectorExpression.h"
#include "VectorIntrinsic.h"

//------------------------------------------------------------------------------
template <typename V, int D>
class VectorT : public VectorDescriptor<V, D> {
public:
	typedef V ValueType;
	typedef ValueType* Iterator;
	typedef const ValueType* ConstIterator;

	enum { Dimension = D };

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
		typename Lhs, 
		typename Rhs, 
		typename Op,
		typename = std::enable_if_t<
			std::is_same<
			typename VectorArgument<Lhs>::ValueType,
			typename VectorArgument<Rhs>::ValueType>::value &&
			(
				VectorArgument<Lhs>::Dimension == VectorArgument<Rhs>::Dimension ||
				VectorArgument<Lhs>::Dimension == 1 ||
				VectorArgument<Rhs>::Dimension == 1
				),
			void>>
	VectorT(const VectorBinaryExpression<Lhs, Rhs, Op>& expr)
	{
		VectorAssignment<
			VectorT, 
			VectorBinaryExpression<Lhs, Rhs, Op>>::Evaluate(*this, expr);
	}

	VectorT& operator = (const VectorT& rhs) { ForEach([&](int i) { m[i] = rhs.m[i]; }); return *this; }

	template <
		typename Lhs, 
		typename Rhs, 
		typename Op,
		typename = std::enable_if_t<
			std::is_same<
			typename VectorArgument<Lhs>::ValueType,
			typename VectorArgument<Rhs>::ValueType>::value &&
			(
				VectorArgument<Lhs>::Dimension == VectorArgument<Rhs>::Dimension ||
				VectorArgument<Lhs>::Dimension == 1 ||
				VectorArgument<Rhs>::Dimension == 1
			),
			void>>
	VectorT& operator = (const VectorBinaryExpression<Lhs, Rhs, Op>& expr) 
	{ 
		VectorAssignment<
			VectorT,
			VectorBinaryExpression<Lhs, Rhs, Op>>::Evaluate(*this, expr);
		return *this;
	}

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

	template <
		typename V,
		typename = std::enable_if_t<
			std::is_same<typename V::ValueType, double>::value,
			void>>
	operator PackedDouble<Dimension>()
	{
		return PackedDouble<Dimension>(*this);
	}
};

//------------------------------------------------------------------------------
// +
struct Add
{
	template <typename Lhs, typename Rhs>
	static const typename Lhs::ValueType Evaluate(const int i, const Lhs& lhs, const Rhs& rhs)
	{ return lhs.Evaluate(i) + rhs.Evaluate(i); }
};

template <typename Lhs, typename Rhs>
const VectorBinaryExpression<Lhs, Rhs, Add> operator + (const Lhs& lhs, const Rhs& rhs)
{ return VectorBinaryExpression<Lhs, Rhs, Add>(lhs, rhs); }

template <typename Lhs>
const VectorBinaryExpression<Lhs, double, Add> operator + (const Lhs& lhs, const double rhs)
{ return VectorBinaryExpression<Lhs, double, Add>(lhs, rhs);  }

template <typename Rhs>
const VectorBinaryExpression<double, Rhs, Add> operator + (const double lhs, const Rhs& rhs)
{ return VectorBinaryExpression<double, Rhs, Add>(lhs, rhs); }

//------------------------------------------------------------------------------
typedef VectorT<float, 2> Vector2F;
typedef VectorT<float, 3> Vector3F;
typedef VectorT<float, 4> Vector4F;

typedef VectorT<double, 2> Vector2D;
typedef VectorT<double, 3> Vector3D;
typedef VectorT<double, 4> Vector4D;

extern void TestVector();

