#pragma once

#include <intrin.h>
#include <immintrin.h>

//------------------------------------------------------------------------------
template <int D>
class PackedDouble {
public:
	enum { Dimension = D };

	PackedDouble() = default;

	PackedDouble(const __m256d& rhs) { p = rhs; }

	template <
		typename V,
		typename = std::enable_if_t<
			std::is_same<typename V::ValueType, double>::value && V::Dimension == Dimension,
			void>>
	PackedDouble(const V& v)
	{
		for (int i = 0; i < V::Dimension; ++i)
		{
			p.m256d_f64[i] = v.m[i];
		}
	}

	__m256d p;
};

