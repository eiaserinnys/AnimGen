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

void TestVector()
{
	Vector2D a(1, 2);
	assert(a.x == 1 && a.y == 2);

	Vector2D b(3, 4);
	Vector2D c(5, 6);
	Vector2D r;

	r = a + b;
	assert(r.x == a.x + b.x && r.y == a.y + b.y);

	r = a + 1.0;
	assert(r.x == a.x + 1.0 && r.y == a.y + 1.0);

	r = 1.0 + a;
	assert(r.x == 1.0 + a.x && r.y == 1.0 + a.y);

	r = a - 1.0;
	assert(r.x == a.x - 1.0 && r.y == a.y - 1.0);

	r = 1.0 - a;
	assert(r.x == 1.0 - a.x && r.y == 1.0 - a.y);

	r = 1.0 + a - 5.0;
	assert(r.x == 1.0 + a.x - 5.0 && r.y == 1.0 + a.y - 5.0);

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

	double dot = Dot(a, b);
	assert(dot == a.x * b.x + a.y * b.y);

	double dot2 = Dot(-a, b);
	assert(dot2 == -a.x * b.x + -a.y * b.y);

	double dot3 = Dot(a - 5.0, b);
	assert(dot3 == (a.x - 5.0) * b.x + (a.y - 5) * b.y);

	double cross = Cross(a, b);
	assert(cross == a.x * b.y - a.y * b.x);

	double len = Length(a);
	assert(len == std::sqrt(a.x * a.x + a.y * a.y));

	double dist = Distance(a, c);
	assert(dist == std::sqrt(Square(a.x - c.x) + Square(a.y - c.y)));

	double dist2 = Distance(a + b, c);
	assert(dist2 == std::sqrt(Square(a.x + b.x - c.x) + Square(a.y + b.y - c.y)));

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