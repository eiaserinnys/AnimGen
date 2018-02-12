#include "pch.h"

#pragma warning(disable:4244)

#include "Vector.h"

#include <assert.h>

using namespace std;
using namespace Core;
using namespace DirectX;

template <typename Scalar>
Scalar Square(Scalar v)
{
	return v * v;
}

template <int D>
const PackedDouble<D> operator + (const PackedDouble<D>& lhs, const PackedDouble<D>& rhs)
{
	return PackedDouble<D>(_mm256_add_pd(lhs.p, rhs.p));
}

//------------------------------------------------------------------------------
static void TestVector2_Arithmetic_Unary()
{
	Vector2D a(1, 2);
	Vector2D b(3, 4);
	Vector2D c(5, 6);
	Vector2D r;

	r = +a;
	assert(r.x == a.x && r.y == a.y);

	r = -a;
	assert(r.x == -a.x && r.y == -a.y);

	r = -(a + b);
	assert(r.x == -(a.x + b.x) && r.y == -(a.y + b.y));

	r = +(a + b / c);
	assert(r.x == +(a.x + b.x / c.x) && r.y == +(a.y + b.y / c.y));

	r = -(a + b / c);
	assert(r.x == -(a.x + b.x / c.x) && r.y == -(a.y + b.y / c.y));
}

//------------------------------------------------------------------------------
static void TestVector2_Arithmetic()
{
	Vector2D a(1, 2);
	Vector2D b(3, 4);
	Vector2D c(5, 6);
	Vector2F d(7.0f, 8.0f);
	Vector2D r;

	//Dot(Vector2D(), Vector3D());

	r = a + b;
	assert((a + b).Dimension == 2);
	assert(r.x == a.x + b.x && r.y == a.y + b.y);

	r = a + 1.0;
	assert(r.x == a.x + 1.0 && r.y == a.y + 1.0);

	r = a + 1;
	assert(r.x == a.x + 1 && r.y == a.y + 1);

	r = 1.0 + a;
	assert(r.x == 1.0 + a.x && r.y == 1.0 + a.y);

	r = a - 1.0;
	assert(r.x == a.x - 1.0 && r.y == a.y - 1.0);

	r = 1.0 - a;
	assert(r.x == 1.0 - a.x && r.y == 1.0 - a.y);

	r = 1.0 + a - 5.0;
	assert(r.x == 1.0 + a.x - 5.0 && r.y == 1.0 + a.y - 5.0);

	r = a + d;
	assert(r.x == a.x + d.x && r.y == a.y + d.y);

	r = d + a;
	assert(r.x == a.x + d.x && r.y == a.y + d.y);

	Vector2F rf;
	rf = a + b;
	assert(rf.x == (float)(a.x + b.x) && rf.y == (float)(a.y + b.y));

	r = a;
	assert(r.x == a.x && r.y == a.y);

	r = a - b;
	assert(r.x == a.x - b.x && r.y == a.y - b.y);

	r = a + b - c;
	assert(r.x == a.x + b.x - c.x && r.y == a.y + b.y - c.y);

	r = a * b;
	assert(r.x == a.x * b.x && r.y == a.y * b.y);

	r = (a + b) * c;
	assert(r.x == (a.x + b.x) * c.x && r.y == (a.y + b.y) * c.y);

	r = a / b;
	assert(r.x == a.x / b.x && r.y == a.y / b.y);

	r = (a + b) / c;
	assert(r.x == (a.x + b.x) / c.x && r.y == (a.y + b.y) / c.y);
}

//------------------------------------------------------------------------------
static void TestVector2_Assignment()
{
	Vector2D a(1, 2);
	assert(a.x == 1 && a.y == 2);
	assert(a.m[0] == 1 && a.m[1] == 2);

	Vector2D b(3, 4);
	Vector2D c(5, 6);
	Vector2D r;

	r = 1.0;

	r = a;
	r += 2.0;
	assert(r.x == a.x + 2.0 && r.y == a.y + 2.0);

	r = a;
	r -= 2.0;
	assert(r.x == a.x - 2.0 && r.y == a.y - 2.0);

	r = a;
	r *= 2.0;
	assert(r.x == a.x * 2.0 && r.y == a.y * 2.0);

	r = a;
	r /= 2.0;
	assert(r.x == a.x / 2.0 && r.y == a.y / 2.0);

	r = a;
	r += b;
	assert(r.x == a.x + b.x && r.y == a.y + b.y);

	r = a;
	r += b + c;
	assert(r.x == a.x + b.x + c.x && r.y == a.y + b.y + c.y);

	r = a;
	r -= b;
	assert(r.x == a.x - b.x && r.y == a.y - b.y);

	r = a;
	r -= b + c;
	assert(r.x == a.x - b.x - c.x && r.y == a.y - b.y - c.y);

	r = a;
	r *= b;
	assert(r.x == a.x * b.x && r.y == a.y * b.y);

	r = a;
	r *= b + c;
	assert(r.x == a.x * (b.x + c.x) && r.y == a.y * (b.y + c.y));

	r = a;
	r /= b;
	assert(r.x == a.x / b.x && r.y == a.y / b.y);

	r = a;
	r /= b + c;
	assert(r.x == a.x / (b.x + c.x) && r.y == a.y / (b.y + c.y));
}

//------------------------------------------------------------------------------
#define TEST_SWIZZLE2(a, b)							\
{													\
	auto a##b = v.a##b();							\
	assert(a##b.x == v.a && a##b.y == v.b);			\
}													\

#define TEST_SWIZZLE3(a, b, c)						\
{													\
	auto a##b##c = v.a##b##c();						\
	assert(a##b##c.x == v.a && a##b##c.y == v.b && a##b##c.z == v.c); \
}													\

#define TEST_SWIZZLE4(a, b, c, d)						\
{														\
	auto a##b##c##d = v.a##b##c##d();					\
	assert(												\
		a##b##c##d.x == v.a && a##b##c##d.y == v.b &&	\
		a##b##c##d.z == v.c && a##b##c##d.w == v.d);	\
}														\

static void TestVector2_Swizzle()
{
	Vector2D v(1, 2);

	TEST_SWIZZLE2(x, x);
	TEST_SWIZZLE2(x, y);
	TEST_SWIZZLE2(y, x);
	TEST_SWIZZLE2(y, y);

	TEST_SWIZZLE3(x, x, x);
	TEST_SWIZZLE3(x, x, y);
	TEST_SWIZZLE3(x, y, x);
	TEST_SWIZZLE3(x, y, y);
	TEST_SWIZZLE3(y, x, x);
	TEST_SWIZZLE3(y, x, y);
	TEST_SWIZZLE3(y, y, x);
	TEST_SWIZZLE3(y, y, y);

	TEST_SWIZZLE4(x, x, x, x);
	TEST_SWIZZLE4(x, x, x, y);
	TEST_SWIZZLE4(x, x, y, x);
	TEST_SWIZZLE4(x, x, y, y);
	TEST_SWIZZLE4(x, y, x, x);
	TEST_SWIZZLE4(x, y, x, y);
	TEST_SWIZZLE4(x, y, y, x);
	TEST_SWIZZLE4(x, y, y, y);
	TEST_SWIZZLE4(y, x, x, x);
	TEST_SWIZZLE4(y, x, x, y);
	TEST_SWIZZLE4(y, x, y, x);
	TEST_SWIZZLE4(y, x, y, y);
	TEST_SWIZZLE4(y, y, x, x);
	TEST_SWIZZLE4(y, y, x, y);
	TEST_SWIZZLE4(y, y, y, x);
	TEST_SWIZZLE4(y, y, y, y);
}

//------------------------------------------------------------------------------
static void TestVector2_Function()
{
	Vector2D a(1, 2);
	Vector2D b(3, 4);
	Vector2D c(5, 6);

	Vector2F d(7.0f, 8.0f);

	double dot = Dot(a, b);
	assert(Dot(a, b).Dimension == 1);
	assert(dot == a.x * b.x + a.y * b.y);

	double dot2 = Dot(-a, b);
	assert(dot2 == -a.x * b.x + -a.y * b.y);

	double dot3 = Dot(a - 5.0, b);
	assert(dot3 == (a.x - 5.0) * b.x + (a.y - 5) * b.y);

	double cross = Cross(a, b);
	assert(cross == a.x * b.y - a.y * b.x);

	double cross2 = Cross(a + b, c);
	assert(cross2 == (a.x + b.x) * c.y - (a.y + b.y) * c.x);

	double len = Length(a);
	assert(Length(a).Dimension == 1);
	assert(len == std::sqrt(a.x * a.x + a.y * a.y));

	double dist = Distance(a, c);
	assert(dist == std::sqrt(Square(a.x - c.x) + Square(a.y - c.y)));

	double dist2 = Distance(a + b, c);
	assert(dist2 == std::sqrt(Square(a.x + b.x - c.x) + Square(a.y + b.y - c.y)));

	Vector2D nor = Normalize(a);
	assert(Normalize(a).Dimension == 2);
	assert(nor.x == a.x / std::sqrt(Square(a.x) + Square(a.y)));
	assert(nor.y == a.y / std::sqrt(Square(a.x) + Square(a.y)));

	Vector2D nor2 = Normalize(a + b);
	assert(nor2.x == (a.x + b.x) / std::sqrt(Square(a.x + b.x) + Square(a.y + b.y)));
	assert(nor2.y == (a.y + b.y) / std::sqrt(Square(a.x + b.x) + Square(a.y + b.y)));

	Vector2D nor3 = Normalize(Normalize(a) + b);
	assert(nor3.x == (nor.x + b.x) / std::sqrt(Square(nor.x + b.x) + Square(nor.y + b.y)));
	assert(nor3.y == (nor.y + b.y) / std::sqrt(Square(nor.x + b.x) + Square(nor.y + b.y)));

	double lf = 0.3;
	Vector2D lerp = Lerp(a, b, lf);
	assert(lerp.x == a.x * (1 - lf) + b.x * lf);
	assert(lerp.y == a.y * (1 - lf) + b.y * lf);
}

//------------------------------------------------------------------------------
static void TestVector2()
{
	TestVector2_Assignment();
	TestVector2_Arithmetic_Unary();
	TestVector2_Arithmetic();
	TestVector2_Swizzle();
	TestVector2_Function();
}

//------------------------------------------------------------------------------
void TestVector()
{
	TestVector2();

	Vector4D a(1, 2, 3, 4);
	auto xxxx = a.xxxx();

#if 0
	// Perform __4__ adds.
	res = _mm256_add_pd(a, b);

	for (int i = 0; i < sizeof(__m256d) / sizeof(double); i++)
	{
		printf("%f + %f = %f\n", a.m256d_f64[i], b.m256d_f64[i], res.m256d_f64[i]);
	}
	puts("");
#endif
}