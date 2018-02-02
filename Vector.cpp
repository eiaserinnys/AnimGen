#include "pch.h"
#include "Vector.h"

#include <assert.h>

using namespace std;
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

static void TestVector2()
{
	Vector2D a(1, 2);
	assert(a.x == 1 && a.y == 2);
	assert(a.m[0] == 1 && a.m[1] == 2);

	Vector2D b(3, 4);
	Vector2D c(5, 6);
	Vector2D r;

	r = a + b;
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

	Vector2F d(7.0f, 8.0f);
	Vector2F rf;

	//auto sw = a.xx();

	r = a + d;
	assert(r.x == a.x + d.x && r.y == a.y + d.y);

	r = d + a;
	assert(r.x == a.x + d.x && r.y == a.y + d.y);

	rf = a + b;
	assert(rf.x == (float) (a.x + b.x) && rf.y == (float)(a.y + b.y));

	r = a;
	assert(r.x == a.x && r.y == a.y);

	r = +a;
	assert(r.x == a.x && r.y == a.y);

	r = -a;
	assert(r.x == - a.x && r.y == - a.y);

	r = - (a + b);
	assert(r.x == -(a.x + b.x) && r.y == -(a.y + b.y));

	r = -(a + b / c);
	assert(r.x == -(a.x + b.x / c.x) && r.y == -(a.y + b.y / c.y));

	r = 1.0;

	r = a;
	r += 1.0;
	assert(r.x == a.x + 1.0 && r.y == a.y + 1.0);

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

	double dot = Dot(a, b);
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
	assert(len == std::sqrt(a.x * a.x + a.y * a.y));

	double dist = Distance(a, c);
	assert(dist == std::sqrt(Square(a.x - c.x) + Square(a.y - c.y)));

	double dist2 = Distance(a + b, c);
	assert(dist2 == std::sqrt(Square(a.x + b.x - c.x) + Square(a.y + b.y - c.y)));

	Vector2D nor = Normalize(a);
	assert(nor.x == a.x / std::sqrt(Square(a.x) + Square(a.y)));
	assert(nor.y == a.y / std::sqrt(Square(a.x) + Square(a.y)));

	Vector2D nor2 = Normalize(a + b);
	assert(nor2.x == (a.x + b.x) / std::sqrt(Square(a.x + b.x) + Square(a.y + b.y)));
	assert(nor2.y == (a.y + b.y) / std::sqrt(Square(a.x + b.x) + Square(a.y + b.y)));

	Vector2D nor3 = Normalize(Normalize(a) + b);
	assert(nor3.x == (nor.x + b.x) / std::sqrt(Square(nor.x + b.x) + Square(nor.y + b.y)));
	assert(nor3.y == (nor.y + b.y) / std::sqrt(Square(nor.x + b.x) + Square(nor.y + b.y)));
}

void TestVector()
{
	TestVector2();

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