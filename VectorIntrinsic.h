#pragma once

#include <intrin.h>
#include <immintrin.h>

#include "VectorMacro.h"

namespace Core
{

	//------------------------------------------------------------------------------
	template <int D>
	class PackedDouble {
	public:
		enum { Dimension = D };

		PackedDouble() = default;

		PackedDouble(const __m256d& rhs) { p = rhs; }

		template <
			typename V,
			ENABLE_IF(
				IS_SAME_TYPE(typename V::ValueType, double) &&
				DIM(V) == Dimension)>
			PackedDouble(const V& v)
		{
			for (int i = 0; i < V::Dimension; ++i)
			{
				p.m256d_f64[i] = v.m[i];
			}
		}

		__m256d p;
	};

};