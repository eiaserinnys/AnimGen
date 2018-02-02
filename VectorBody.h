#pragma once

#include "VectorExpression.h"
#include "VectorOperation.h"

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

	__forceinline VectorT(const VectorT& rhs) { operator = (rhs); }

	template <typename ValueType, ENABLE_IF(Dimension == 2)>
	__forceinline VectorT(ValueType x, ValueType y)
	{ m[0] = x; m[1] = y; }

	template <typename ValueType, ENABLE_IF(Dimension == 3)>
	__forceinline VectorT(ValueType x, ValueType y, ValueType z)
	{ m[0] = x; m[1] = y; m[2] = z; }

	template <typename ValueType, ENABLE_IF(Dimension == 4)>
	__forceinline VectorT(ValueType x, ValueType y, ValueType z, ValueType w)
	{ m[0] = x; m[1] = y; m[2] = z; m[3] = w; }

	template <typename Expr>
	__forceinline VectorT(const Expr& expr)
	{ 
		using namespace VectorOperation;
		VectorAssignment<VectorT, Expr, SelectRhs>().Evaluate(*this, expr);
	}

	//--------------------------------------------------------------------------
	// assignment
	template<typename Expr>
	using AssignSrc = VectorUnaryExpression<Expr, VectorOperation::Plus>;

	template<typename Expr, typename Op>
	using AssignExpr = VectorAssignment<VectorT, AssignSrc<Expr>, Op>;

	template <typename Expr>
	__forceinline VectorT& operator = (const Expr& expr)
	{
		using namespace VectorOperation;
		AssignExpr<Expr, SelectRhs>().Evaluate(*this, AssignSrc<Expr>(expr));
		return *this;
	}

	template <typename Expr>
	__forceinline VectorT& operator += (const Expr& expr)
	{
		using namespace VectorOperation;
		AssignExpr<Expr, Add>().Evaluate(*this, AssignSrc<Expr>(expr));
		return *this;
	}

	template <typename Expr>
	__forceinline VectorT& operator -= (const Expr& expr)
	{
		using namespace VectorOperation;
		AssignExpr<Expr, Subtract>().Evaluate(*this, AssignSrc<Expr>(expr));
		return *this;
	}

	template <typename Expr>
	__forceinline VectorT& operator *= (const Expr& expr)
	{
		using namespace VectorOperation;
		AssignExpr<Expr, Multiply>().Evaluate(*this, AssignSrc<Expr>(expr));
		return *this;
	}

	template <typename Expr>
	__forceinline VectorT& operator /= (const Expr& expr)
	{
		using namespace VectorOperation;
		AssignExpr<Expr, Divide>().Evaluate(*this, AssignSrc<Expr>(expr));
		return *this;
	}

	//--------------------------------------------------------------------------
	// template meta expression evaluation
	__forceinline void PreEvaluate() const {}
	__forceinline const ValueType Evaluate(int index) const { return m[index]; }

	//--------------------------------------------------------------------------
	// swizzle
#	define HAS_MEMBER_x ENABLE_IF(Dimension > 0)
#	define HAS_MEMBER_y ENABLE_IF(Dimension > 1)
#	define HAS_MEMBER_z ENABLE_IF(Dimension > 2)
#	define HAS_MEMBER_w ENABLE_IF(Dimension > 3)

#	define HAS_MEMBER(a) HAS_MEMBER_##a

#	define SWIZZLE_2(a, b, c, d) \
	template <HAS_MEMBER(a), HAS_MEMBER(b)> \
		__forceinline const VectorT<V, 2> a##b() const { return VectorT<V, 2>(a, b); }

#	define SWIZZLE_3(a, b, c, d) \
	template <HAS_MEMBER(a), HAS_MEMBER(b), HAS_MEMBER(c)> \
		__forceinline const VectorT<V, 3> a##b##c() const { return VectorT<V, 3>(a, b, c); }

#	define SWIZZLE_4(a, b, c, d) \
	template <HAS_MEMBER(a), HAS_MEMBER(b), HAS_MEMBER(c), HAS_MEMBER(c)> \
		__forceinline const VectorT<V, 4> a##b##c##d() const { return VectorT<V, 3>(a, b, c, d); }

#	define SWIZZLE5(n, a, b, c, d) SWIZZLE_##n(a, b, c, d)

#	define SWIZZLE4(n, a, b, c) \
	SWIZZLE5(n, a, b, c, x); SWIZZLE5(n, a, b, c, y); SWIZZLE5(n, a, b, c, z); SWIZZLE5(n, a, b, c, w);

#	define SWIZZLE3(n, a, b) \
	SWIZZLE4(n, a, b, x); SWIZZLE4(n, a, b, y); SWIZZLE4(n, a, b, z); SWIZZLE4(n, a, b, w);

#	define SWIZZLE2(n, a) \
	SWIZZLE3(n, a, x); SWIZZLE3(n, a, y); SWIZZLE3(n, a, z); SWIZZLE3(n, a, w);

#	define SWIZZLE(n) \
	SWIZZLE2(n, x); SWIZZLE2(n, y); SWIZZLE2(n, z); SWIZZLE2(n, w);

	//SWIZZLE(2);
	//SWIZZLE(3);
	SWIZZLE(4);
};
