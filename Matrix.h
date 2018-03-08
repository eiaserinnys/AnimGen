#pragma once

namespace Core
{

	template <typename V, int M, int N>
	class MatrixT {
	public:
		typedef V ValueType;

		template <ENABLE_IF(M == N)>
		static MatrixT Identity()
		{
			MatrixT ret;
			for (int i = 0; i < M; ++i)
			{
				for (int j = 0; j < N; ++j)
				{
					ret.m[i][j] = i == j ? 1 : 0;
				}
			}
			return ret;
		}

		template <ENABLE_IF(M == 4 && N == 4)>
		MatrixT Inverse(ValueType* detOut = nullptr) const
		{
			ValueType inv[16], det;

			inv[0] = e[5] * e[10] * e[15] -
				e[5] * e[11] * e[14] -
				e[9] * e[6] * e[15] +
				e[9] * e[7] * e[14] +
				e[13] * e[6] * e[11] -
				e[13] * e[7] * e[10];

			inv[4] = -e[4] * e[10] * e[15] +
				e[4] * e[11] * e[14] +
				e[8] * e[6] * e[15] -
				e[8] * e[7] * e[14] -
				e[12] * e[6] * e[11] +
				e[12] * e[7] * e[10];

			inv[8] = e[4] * e[9] * e[15] -
				e[4] * e[11] * e[13] -
				e[8] * e[5] * e[15] +
				e[8] * e[7] * e[13] +
				e[12] * e[5] * e[11] -
				e[12] * e[7] * e[9];

			inv[12] = -e[4] * e[9] * e[14] +
				e[4] * e[10] * e[13] +
				e[8] * e[5] * e[14] -
				e[8] * e[6] * e[13] -
				e[12] * e[5] * e[10] +
				e[12] * e[6] * e[9];

			inv[1] = -e[1] * e[10] * e[15] +
				e[1] * e[11] * e[14] +
				e[9] * e[2] * e[15] -
				e[9] * e[3] * e[14] -
				e[13] * e[2] * e[11] +
				e[13] * e[3] * e[10];

			inv[5] = e[0] * e[10] * e[15] -
				e[0] * e[11] * e[14] -
				e[8] * e[2] * e[15] +
				e[8] * e[3] * e[14] +
				e[12] * e[2] * e[11] -
				e[12] * e[3] * e[10];

			inv[9] = -e[0] * e[9] * e[15] +
				e[0] * e[11] * e[13] +
				e[8] * e[1] * e[15] -
				e[8] * e[3] * e[13] -
				e[12] * e[1] * e[11] +
				e[12] * e[3] * e[9];

			inv[13] = e[0] * e[9] * e[14] -
				e[0] * e[10] * e[13] -
				e[8] * e[1] * e[14] +
				e[8] * e[2] * e[13] +
				e[12] * e[1] * e[10] -
				e[12] * e[2] * e[9];

			inv[2] = e[1] * e[6] * e[15] -
				e[1] * e[7] * e[14] -
				e[5] * e[2] * e[15] +
				e[5] * e[3] * e[14] +
				e[13] * e[2] * e[7] -
				e[13] * e[3] * e[6];

			inv[6] = -e[0] * e[6] * e[15] +
				e[0] * e[7] * e[14] +
				e[4] * e[2] * e[15] -
				e[4] * e[3] * e[14] -
				e[12] * e[2] * e[7] +
				e[12] * e[3] * e[6];

			inv[10] = e[0] * e[5] * e[15] -
				e[0] * e[7] * e[13] -
				e[4] * e[1] * e[15] +
				e[4] * e[3] * e[13] +
				e[12] * e[1] * e[7] -
				e[12] * e[3] * e[5];

			inv[14] = -e[0] * e[5] * e[14] +
				e[0] * e[6] * e[13] +
				e[4] * e[1] * e[14] -
				e[4] * e[2] * e[13] -
				e[12] * e[1] * e[6] +
				e[12] * e[2] * e[5];

			inv[3] = -e[1] * e[6] * e[11] +
				e[1] * e[7] * e[10] +
				e[5] * e[2] * e[11] -
				e[5] * e[3] * e[10] -
				e[9] * e[2] * e[7] +
				e[9] * e[3] * e[6];

			inv[7] = e[0] * e[6] * e[11] -
				e[0] * e[7] * e[10] -
				e[4] * e[2] * e[11] +
				e[4] * e[3] * e[10] +
				e[8] * e[2] * e[7] -
				e[8] * e[3] * e[6];

			inv[11] = -e[0] * e[5] * e[11] +
				e[0] * e[7] * e[9] +
				e[4] * e[1] * e[11] -
				e[4] * e[3] * e[9] -
				e[8] * e[1] * e[7] +
				e[8] * e[3] * e[5];

			inv[15] = e[0] * e[5] * e[10] -
				e[0] * e[6] * e[9] -
				e[4] * e[1] * e[10] +
				e[4] * e[2] * e[9] +
				e[8] * e[1] * e[6] -
				e[8] * e[2] * e[5];

			det = e[0] * inv[0] + e[1] * inv[4] + e[2] * inv[8] + e[3] * inv[12];

			if (det == 0) { return MatrixT::Identity(); }

			det = (ValueType)1 / det;

			MatrixT invOut;

			for (int i = 0; i < 16; i++)
			{
				invOut.e[i] = inv[i] * det;
			}

			return invOut;
		}

		typedef ValueType Row[N];
		union
		{
			ValueType e[M * N];
			Row r[M];
			ValueType m[M][N];
		};

		bool AssertEqual(const MatrixT<V, M, N>& rhs, V epsilon = 0.01) const
		{
			for (int i = 0; i < M * N; ++i)
			{
				if (abs(e[i] - rhs.e[i]) >= epsilon)
				{
					return false;
				}
			}
			return true;
		}
	};

	template <typename V, int M, int N, ENABLE_IF(M == N)>
	inline MatrixT<V, M, N> operator * (
		const MatrixT<V, M, N>& M1,
		const MatrixT<V, M, N>& M2)
	{
		MatrixT<V, M, N> mResult;
		// Cache the invariants in registers
		V x = M1.m[0][0];
		V y = M1.m[0][1];
		V z = M1.m[0][2];
		V w = M1.m[0][3];
		// Perform the operation on the first row
		mResult.m[0][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[0][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[0][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[0][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		// Repeat for all the other rows
		x = M1.m[1][0];
		y = M1.m[1][1];
		z = M1.m[1][2];
		w = M1.m[1][3];
		mResult.m[1][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[1][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[1][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[1][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		x = M1.m[2][0];
		y = M1.m[2][1];
		z = M1.m[2][2];
		w = M1.m[2][3];
		mResult.m[2][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[2][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[2][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[2][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		x = M1.m[3][0];
		y = M1.m[3][1];
		z = M1.m[3][2];
		w = M1.m[3][3];
		mResult.m[3][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[3][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[3][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[3][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		return mResult;
	}

	typedef MatrixT<double, 4, 4> Matrix4D;

};