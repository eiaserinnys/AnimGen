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
};
