#pragma once

#include "VectorExpression.h"
#include "VectorOperation.h"

namespace Core
{
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

		template <
			typename X, typename Y, 
			ENABLE_IF(
				Dimension == 2 && 
				IS_CONVERTIBLE(X, ValueType) &&
				IS_CONVERTIBLE(Y, ValueType))>
		__forceinline VectorT(X x, Y y)
		{
			m[0] = x; m[1] = y;
		}

		template <
			typename X, typename Y, typename Z,
			ENABLE_IF(
				Dimension == 3 && 
				IS_CONVERTIBLE(X, ValueType) &&
				IS_CONVERTIBLE(Y, ValueType) &&
				IS_CONVERTIBLE(Z, ValueType))>
		__forceinline VectorT(X x, Y y, Z z)
		{
			m[0] = x; m[1] = y; m[2] = z;
		}

		template <
			typename X, typename Y, typename Z, typename W, 
			ENABLE_IF(
				Dimension == 4 && 
				IS_CONVERTIBLE(X, ValueType) &&
				IS_CONVERTIBLE(Y, ValueType) &&
				IS_CONVERTIBLE(Z, ValueType) && 
				IS_CONVERTIBLE(W, ValueType))>
		__forceinline VectorT(X x, Y y, Z z, W w)
		{
			m[0] = x; m[1] = y; m[2] = z; m[3] = w;
		}

		template <typename Expr>
		__forceinline VectorT(const Expr& expr)
		{
			using namespace VectorOperator;
			VectorAssignment<SelectRhs<VectorT, Expr>>().Evaluate(*this, expr);
		}

		//--------------------------------------------------------------------------
		// assignment
		template <typename Expr>
		__forceinline VectorT& operator = (const Expr& expr)
		{
			using namespace VectorOperator;
			VectorAssignment<SelectRhs<VectorT, Expr>>().Evaluate(*this, expr);
			return *this;
		}

		template <typename Expr>
		__forceinline VectorT& operator += (const Expr& expr)
		{
			using namespace VectorOperator;
			VectorAssignment<Add<VectorT, Expr>>().Evaluate(*this, expr);
			return *this;
		}

		template <typename Expr>
		__forceinline VectorT& operator -= (const Expr& expr)
		{
			using namespace VectorOperator;
			VectorAssignment<Subtract<VectorT, Expr>>().Evaluate(*this, expr);
			return *this;
		}

		template <typename Expr>
		__forceinline VectorT& operator *= (const Expr& expr)
		{
			using namespace VectorOperator;
			VectorAssignment<Multiply<VectorT, Expr>>().Evaluate(*this, expr);
			return *this;
		}

		template <typename Expr>
		__forceinline VectorT& operator /= (const Expr& expr)
		{
			using namespace VectorOperator;
			VectorAssignment<Divide<VectorT, Expr>>().Evaluate(*this, expr);
			return *this;
		}

		//--------------------------------------------------------------------------
		template <typename X, ENABLE_IF(IS_CONVERTIBLE(X, ValueType))>
		void Fill(X x) 
		{ for (int i = 0; i < Dimension; ++i) { m[i] = x; } }

		void FillZero() { Fill(0); }

		//--------------------------------------------------------------------------
		// template meta expression evaluation
		__forceinline void PreEvaluate() const {}
		__forceinline const ValueType Evaluate(int index) const { return m[index]; }

		//--------------------------------------------------------------------------
		// swizzle
#		define HAS_MEMBER_x ENABLE_IF(Dimension > 0)
#		define HAS_MEMBER_y ENABLE_IF(Dimension > 1)
#		define HAS_MEMBER_z ENABLE_IF(Dimension > 2)
#		define HAS_MEMBER_w ENABLE_IF(Dimension > 3)

#		define HAS_MEMBER(a) HAS_MEMBER_##a

	// swizzler with the name like 'xx', 'xy'
#		define SWIZZLE() \
		SWIZZLE_1(x); SWIZZLE_1(y); SWIZZLE_1(z); SWIZZLE_1(w);

#		define SWIZZLE_1(a) \
		SWIZZLE_2(a, x); SWIZZLE_2(a, y); SWIZZLE_2(a, z); SWIZZLE_2(a, w);

#		define SWIZZLE_2(a, b) \
		template <HAS_MEMBER(a), HAS_MEMBER(b)> \
		__forceinline const VectorT<V, 2> a##b() const { return VectorT<V, 2>(a, b); }

		SWIZZLE();

		// swizzler with the name like 'xxx', 'xyz'
#		undef SWIZZLE_2
#		define SWIZZLE_2(a, b) \
		SWIZZLE_3(a, b, x); SWIZZLE_3(a, b, y); SWIZZLE_3(a, b, z); SWIZZLE_3(a, b, w);

#		define SWIZZLE_3(a, b, c) \
		template <HAS_MEMBER(a), HAS_MEMBER(b), HAS_MEMBER(c)> \
		__forceinline const VectorT<V, 3> a##b##c() const { return VectorT<V, 3>(a, b, c); }

		SWIZZLE();

		// swizzler with the name like 'xxxx', 'xyzw'
#		undef SWIZZLE_3
#		define SWIZZLE_3(a, b, c) \
		SWIZZLE_4(a, b, c, x); SWIZZLE_4(a, b, c, y); SWIZZLE_4(a, b, c, z); SWIZZLE_4(a, b, c, w);

#		define SWIZZLE_4(a, b, c, d) \
		template <HAS_MEMBER(a), HAS_MEMBER(b), HAS_MEMBER(c), HAS_MEMBER(c)> \
		__forceinline const VectorT<V, 4> a##b##c##d() const { return VectorT<V, 4>(a, b, c, d); }

		SWIZZLE();
	};

};