#include "pch.h"
#include "Vector.h"

using namespace std;
using namespace DirectX;

template <int D>
const PackedDouble<D> operator + (const PackedDouble<D>& lhs, const PackedDouble<D>& rhs)
{
	return PackedDouble<D>(_mm256_add_pd(lhs.p, rhs.p));
}

void TestVector()
{
	Vector2D a2(1, 2);
	Vector2D b2(3, 4);

	Vector2D av;

	av = a2 + a2 + b2;

	av = a2 + 1.0;

	av = 1.0 + a2;

	av = 1.0 + a2 - 5.0;

	av = +a2;

	av = -a2;

	av = - (a2 + b2);

	auto d = Dot(a2, b2);
	auto dr = d.Evaluate(0);

	dr = Dot(a2, b2);

	dr = Dot(a2 + b2, b2);

	double c = Cross(a2, b2);

	Vector3D ccc = Cross(Vector3D(1, 2, 3), Vector3D(4, 5, 6));

	XMVECTOR cv = XMVector3Cross(
		XMLoadFloat3(&XMFLOAT3(1, 2, 3)),
		XMLoadFloat3(&XMFLOAT3(4, 5, 6)));


#if 0
	bool f2d = is_convertible<float, double>::value;
	bool d2f = is_convertible<float, double>::value;

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
#endif
}