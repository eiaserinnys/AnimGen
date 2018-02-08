#pragma once

#include "Vector.h"
#include "Matrix.h"

namespace Core
{
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

		template <int N, int M>
		static inline VectorT<V, M> Transform(
			const VectorT<V, N>& v,
			const MatrixT<V, N, M>& m)
		{
			VectorT<V, M> ret;
			ret.FillZero();

			for (int i = 0; i < M; ++i)
			{
				for (int j = 0; j < N; ++j)
				{
					ret.m[i] += v.m[j] * m.m[j][i];
				}
			}

			return ret;
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
			return Matrix
			{
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				x, y, z, 1,
			};
		}

		static inline Matrix RotationZ(ValueType Angle)
		{
			ValueType s = std::sin(Angle);
			ValueType c = std::cos(Angle);

			return Matrix
			{
				c, s, 0, 0, 
				-s, c, 0, 0, 
				0, 0, 1, 0, 
				0, 0, 0, 1, 
			};
		}

		static inline Matrix RotationY(float Angle)
		{
			ValueType s = std::sin(Angle);
			ValueType c = std::cos(Angle);

			return Matrix
			{
				c, 0, -s, 0,
				0, 1, 0, 0,
				s, 0, c, 0,
				0, 0, 0, 1,
			};
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

		static inline MatrixT<V, 4, 4> MatrixRotationQuaternion(const VectorT<V, 4>& q)
		{
			V qx = q.x, qy = q.y, qz = q.z, qw = q.w;
			V qx2 = q.x* q.x, qy2 = q.y * q.y, qz2 = q.z* q.z, qw2 = q.w * q.w;

			MatrixT<V, 4, 4> m =
			{
				1 - 2 * (qy2 + qz2), 2 * (qx * qy + qz * qw), 2 * (qx * qz - qy * qw), 0,
				2 * (qx * qy - qz * qw), 1 - 2 * (qx2 + qz2), 2 * (qy * qz + qx * qw), 0,
				2 * (qx * qz + qy * qw), 2 * (qy * qz - qx * qw), 1 - 2 * (qx2 + qy2), 0,
				0, 0, 0, 1
			};

			return m;
		}

		static inline VectorT<V, 4> QuaternionConjugate(const VectorT<V, 4>& quat)
		{
			return VectorT<V, 4>(-quat.x, -quat.y, -quat.z, quat.w);
		}

		static inline VectorT<V, 4> QuaternionInverse(const VectorT<V, 4>& quat)
		{
			return Normalize(QuaternionConjugate(quat));
		}

		static inline VectorT<V, 4> QuaternionMultiply(
			const VectorT<V, 4>& Q1, 
			const VectorT<V, 4>& Q2)
		{
			return VectorT<V, 4>
			{
				(Q2.w * Q1.x) + (Q2.x * Q1.w) + (Q2.y * Q1.z) - (Q2.z * Q1.y),
				(Q2.w * Q1.y) - (Q2.x * Q1.z) + (Q2.y * Q1.w) + (Q2.z * Q1.x),
				(Q2.w * Q1.z) + (Q2.x * Q1.y) - (Q2.y * Q1.x) + (Q2.z * Q1.w),
				(Q2.w * Q1.w) - (Q2.x * Q1.x) - (Q2.y * Q1.y) - (Q2.z * Q1.z)
			};
		}
	};
}