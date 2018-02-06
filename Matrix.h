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
	};

	template <typename V, int M, int N, ENABLE_IF(M == N)>
	inline MatrixT<V, M, N> operator * (
		const MatrixT<V, M, N>& M1,
		const MatrixT<V, M, N>& M2)
	{
		MatrixT<V, M, N> mResult;
		// Cache the invariants in registers
		float x = M1.m[0][0];
		float y = M1.m[0][1];
		float z = M1.m[0][2];
		float w = M1.m[0][3];
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

	template <typename V>
	struct DXMathTransform
	{
		typedef V ValueType;
		typedef MatrixT<V, 4, 4> Matrix;

		static inline VectorT<V, 3> Transform(
			const VectorT<V, 3>& v,
			const MatrixT<V, 4, 4>& m)
		{
			return VectorT<V, 3>(
				v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0],
				v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1],
				v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2]);
		}

		static inline VectorT<V, 3> TransformNormal(
			const VectorT<V, 3>& v,
			const MatrixT<V, 4, 4>& m)
		{
			return VectorT<V, 3>(
				v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
				v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
				v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]);
		}

		static inline Matrix Translation(ValueType x, ValueType y, ValueType z)
		{
			Matrix m;
			m.m[0][0] = 1; m.m[0][1] = 0; m.m[0][2] = 0; m.m[0][3] = 0;
			m.m[1][0] = 0; m.m[1][1] = 1; m.m[1][2] = 0; m.m[1][3] = 0;
			m.m[2][0] = 0; m.m[2][1] = 0; m.m[2][2] = 1; m.m[2][3] = 0;

			m.m[3][0] = x;
			m.m[3][1] = y;
			m.m[3][2] = z;
			m.m[3][3] = 1;
			return m;
		}

		static inline Matrix RotationZ(ValueType Angle)
		{
			ValueType fSinAngle = std::sin(Angle);
			ValueType fCosAngle = std::cos(Angle);

			Matrix M;
			M.m[0][0] = fCosAngle;
			M.m[0][1] = fSinAngle;
			M.m[0][2] = 0.0f;
			M.m[0][3] = 0.0f;

			M.m[1][0] = -fSinAngle;
			M.m[1][1] = fCosAngle;
			M.m[1][2] = 0.0f;
			M.m[1][3] = 0.0f;

			M.m[2][0] = 0.0f;
			M.m[2][1] = 0.0f;
			M.m[2][2] = 1.0f;
			M.m[2][3] = 0.0f;

			M.m[3][0] = 0.0f;
			M.m[3][1] = 0.0f;
			M.m[3][2] = 0.0f;
			M.m[3][3] = 1.0f;
			return M;
		}

		static inline Matrix RotationY(float Angle)
		{
			ValueType fSinAngle = std::sin(Angle);
			ValueType fCosAngle = std::cos(Angle);

			Matrix M;
			M.m[0][0] = fCosAngle;
			M.m[0][1] = 0.0f;
			M.m[0][2] = -fSinAngle;
			M.m[0][3] = 0.0f;

			M.m[1][0] = 0.0f;
			M.m[1][1] = 1.0f;
			M.m[1][2] = 0.0f;
			M.m[1][3] = 0.0f;

			M.m[2][0] = fSinAngle;
			M.m[2][1] = 0.0f;
			M.m[2][2] = fCosAngle;
			M.m[2][3] = 0.0f;

			M.m[3][0] = 0.0f;
			M.m[3][1] = 0.0f;
			M.m[3][2] = 0.0f;
			M.m[3][3] = 1.0f;
			return M;
		}

		static inline VectorT<V, 4> QuaternionRotationMatrix(const Matrix& M)
		{
			VectorT<V, 4> q;
			ValueType r22 = M.m[2][2];
			if (r22 <= 0)  // x^2 + y^2 >= z^2 + w^2
			{
				ValueType dif10 = M.m[1][1] - M.m[0][0];
				ValueType omr22 = (ValueType)1 - r22;
				if (dif10 <= 0)  // x^2 >= y^2
				{
					ValueType fourXSqr = omr22 - dif10;
					ValueType inv4x = 0.5 / std::sqrt(fourXSqr);
					q.m[0] = fourXSqr*inv4x;
					q.m[1] = (M.m[0][1] + M.m[1][0])*inv4x;
					q.m[2] = (M.m[0][2] + M.m[2][0])*inv4x;
					q.m[3] = (M.m[1][2] - M.m[2][1])*inv4x;
				}
				else  // y^2 >= x^2
				{
					ValueType fourYSqr = omr22 + dif10;
					ValueType inv4y = 0.5 / std::sqrt(fourYSqr);
					q.m[0] = (M.m[0][1] + M.m[1][0])*inv4y;
					q.m[1] = fourYSqr*inv4y;
					q.m[2] = (M.m[1][2] + M.m[2][1])*inv4y;
					q.m[3] = (M.m[2][0] - M.m[0][2])*inv4y;
				}
			}
			else  // z^2 + w^2 >= x^2 + y^2
			{
				ValueType sum10 = M.m[1][1] + M.m[0][0];
				ValueType opr22 = (ValueType)1 + r22;
				if (sum10 <= 0)  // z^2 >= w^2
				{
					ValueType fourZSqr = opr22 - sum10;
					ValueType inv4z = 0.5 / std::sqrt(fourZSqr);
					q.m[0] = (M.m[0][2] + M.m[2][0])*inv4z;
					q.m[1] = (M.m[1][2] + M.m[2][1])*inv4z;
					q.m[2] = fourZSqr*inv4z;
					q.m[3] = (M.m[0][1] - M.m[1][0])*inv4z;
				}
				else  // w^2 >= z^2
				{
					ValueType fourWSqr = opr22 + sum10;
					ValueType inv4w = 0.5 / std::sqrt(fourWSqr);
					q.m[0] = (M.m[1][2] - M.m[2][1])*inv4w;
					q.m[1] = (M.m[2][0] - M.m[0][2])*inv4w;
					q.m[2] = (M.m[0][1] - M.m[1][0])*inv4w;
					q.m[3] = fourWSqr*inv4w;
				}
			}
			return q;
		}
	};

	typedef MatrixT<double, 4, 4> Matrix4D;

};