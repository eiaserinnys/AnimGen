#pragma once

#include <type_traits>
#include <functional>

#define ENABLE_IF(x) typename = std::enable_if_t<(x), void>
#define DIM(x) x::Dimension
#define HAS_SAME_DIMENSION(x, y) (DIM(x) == DIM(y))
#define IS_SCALAR(x) (x::Dimension == 1)
#define IS_SAME_TYPE(x, y) std::is_same<x, y>::value
#define IS_CONVERTIBLE(x, y) std::is_convertible<x, y>::value
#define DECLVAL(x) std::declval<typename x::ValueType>()

namespace Core
{

	template <typename A, typename B>
	struct MoreGeneralType {};

	template <> struct MoreGeneralType<int, int> { typedef int Type; };
	template <> struct MoreGeneralType<int, float> { typedef float Type; };
	template <> struct MoreGeneralType<int, double> { typedef double Type; };

	template <> struct MoreGeneralType<float, int> { typedef float Type; };
	template <> struct MoreGeneralType<float, float> { typedef float Type; };
	template <> struct MoreGeneralType<float, double> { typedef double Type; };

	template <> struct MoreGeneralType<double, int> { typedef double Type; };
	template <> struct MoreGeneralType<double, float> { typedef double Type; };
	template <> struct MoreGeneralType<double, double> { typedef double Type; };

	template <int A, int B>
	struct Max { enum { Value = A > B ? A : B }; };

};

#define MORE_GENERAL_TYPE(x, y) typename MoreGeneralType<x, y>::Type
#define MORE_GENERAL_TYPE_3(x, y, z) typename MoreGeneralType<x, MoreGeneralType<x, y>::Type>::Type

