#include "pch.h"
#include "Vector.h"

using namespace std;

template <int D>
const PackedDouble<D> operator + (const PackedDouble<D>& lhs, const PackedDouble<D>& rhs)
{
	return PackedDouble<D>(_mm256_add_pd(lhs.p, rhs.p));
}

void TestVector()
{
	Vector2D a2(1, 2);
	Vector2D b2(3, 4);

	Vector2F f2(5, 7);

	PackedDouble<2> p = a2;

	auto add = a2 + b2;

	auto s1 = add.Evaluate(0);
	auto s2 = add.Evaluate(1);

	Vector2D av = add;

	av = a2 + a2 + b2;

	av = a2 + 1.0;

	av = 1.0 + a2;

	av = 1.0 + a2 + 1.0;

	//a2 + b2;

#if 0

	Vector2D c2;

	c2 += b2;
#endif

	Vector3D a3(1, 2, 3);

	Vector4D a4(1, 2, 3, 4);



	__m256d a, b, res;

	for (int i = 0; i < sizeof(__m256d) / sizeof(double); i++)
	{
		a.m256d_f64[i] = i;
		b.m256d_f64[i] = 2 * i;
	}

	// Perform __4__ adds.
	res = _mm256_add_pd(a, b);

	for (int i = 0; i < sizeof(__m256d) / sizeof(double); i++)
	{
		printf("%f + %f = %f\n", a.m256d_f64[i], b.m256d_f64[i], res.m256d_f64[i]);
	}
	puts("");
}