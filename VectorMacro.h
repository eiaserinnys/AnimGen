#pragma once

#include <type_traits>
#include <functional>

#define ENABLE_IF(x) typename = std::enable_if_t<(x), void>
#define DIM(x) x::Dimension
#define HAS_SAME_DIMENSION(x, y) (DIM(x) == DIM(y))
#define IS_SCALAR(x) (x::Dimension == 1)
#define IS_CONVERTIBLE(x, y) std::is_convertible<x, y>::value

template <typename A, typename B>
struct MoreGenericType {};

template <> struct MoreGenericType<int, int> { typedef int Type; };
template <> struct MoreGenericType<int, float> { typedef float Type; };
template <> struct MoreGenericType<int, double> { typedef double Type; };

template <> struct MoreGenericType<float, int> { typedef float Type; };
template <> struct MoreGenericType<float, float> { typedef float Type; };
template <> struct MoreGenericType<float, double> { typedef double Type; };

template <> struct MoreGenericType<double, int> { typedef double Type; };
template <> struct MoreGenericType<double, float> { typedef double Type; };
template <> struct MoreGenericType<double, double> { typedef double Type; };

