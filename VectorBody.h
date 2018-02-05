#pragma once

#include "VectorExpression.h"
#include "VectorOperation.h"

namespace Core
{

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
		{
			m[0] = x; m[1] = y;
		}

		template <typename ValueType, ENABLE_IF(Dimension == 3)>
		__forceinline VectorT(ValueType x, ValueType y, ValueType z)
		{
			m[0] = x; m[1] = y; m[2] = z;
		}

		template <typename ValueType, ENABLE_IF(Dimension == 4)>
		__forceinline VectorT(ValueType x, ValueType y, ValueType z, ValueType w)
		{
			m[0] = x; m[1] = y; m[2] = z; m[3] = w;
		}

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