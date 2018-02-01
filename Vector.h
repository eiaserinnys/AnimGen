#pragma once

#include <type_traits>
#include <functional>

#include "VectorDescriptor.h"
#include "VectorExpression.h"
#include "VectorBody.h"
#include "VectorOperation.h"
#include "VectorIntrinsic.h"

typedef VectorT<float, 2> Vector2F;
typedef VectorT<float, 3> Vector3F;
typedef VectorT<float, 4> Vector4F;

typedef VectorT<double, 2> Vector2D;
typedef VectorT<double, 3> Vector3D;
typedef VectorT<double, 4> Vector4D;

extern void TestVector();

