#include <Nubuck\math\vector3.h>
#include <Nubuck\math\quaternion.h>
#include <Nubuck\math\matrix3.h>

namespace M {

	Matrix3 operator*(const Matrix3& lhp, const Matrix3& rhp) {
		Matrix3 mat;
		for(int i = 0; i < 3; ++i) {
			for(int j = 0; j < 3; ++j) {
				float el = 0.0f;
				for(int k = 0; k < 3; ++k) {
					el += lhp.mat[i + 3 * k] * rhp.mat[k + 3 * j];
				}
				mat.mat[i + 3 * j] = el;
			}
		}
		return mat;
	}

	Matrix3 operator/(const Matrix3& mat, float scalar) {
		return Matrix3(mat.m00 / scalar, mat.m01 / scalar, mat.m02 / scalar,
					   mat.m10 / scalar, mat.m11 / scalar, mat.m12 / scalar,
					   mat.m20 / scalar, mat.m21 / scalar, mat.m22 / scalar);
	}

	Vector3 Transform(const Matrix3& mat, const Vector3& vec) {
		Vector3 ret;
		for(int i = 0; i < 3; ++i) {
			float el = 0.0f;
			for(int j = 0; j < 3; ++j) {
				el += mat.mat[i + 3 * j] * vec.vec[j];
			}
			ret.vec[i] = el;
		}
		return ret;
	}

	float Det(const Matrix3& m) {
		return m.a * (m.e * m.i - m.f * m.h)
			- m.b * (m.d * m.i - m.f * m.g)
			+ m.c * (m.d * m.h - m.e * m.g);
	}

    Matrix3 Transpose(const Matrix3& m) {
		return Matrix3(m.m00, m.m10, m.m20,
					   m.m01, m.m11, m.m21,
					   m.m02, m.m12, m.m22);
    }

	Matrix3 Inverse(const Matrix3& m, const float det) {
		Matrix3 inv(m.e * m.i - m.f * m.h, m.c * m.h - m.b * m.i, m.b * m.f - m.c * m.e,
					m.f * m.g - m.d * m.i, m.a * m.i - m.c * m.g, m.c * m.d - m.a * m.f,
					m.d * m.h - m.e * m.g, m.b * m.g - m.a * m.h, m.a * m.e - m.b * m.d);
		return inv / det;
	}

	Matrix3 Inverse(const Matrix3& m) {
		return Inverse(m, Det(m));
	}

	Matrix3 RigidInverse(const Matrix3& m) {
        return Transpose(m);
	}

	void ToColumns(const M::Matrix3& mat, M::Vector3& A1, M::Vector3& A2, M::Vector3& A3) {
		A1.x = mat.m00;
		A1.y = mat.m10;
		A1.z = mat.m20;

		A2.x = mat.m01;
		A2.y = mat.m11;
		A2.z = mat.m21;

		A3.x = mat.m02;
		A3.y = mat.m12;
		A3.z = mat.m22;
	}

    void Orthonormalize(Matrix3& mat) {
        Vector3 v0, v1, v2;
        ToColumns(mat, v0, v1, v2);
        Orthogonalize(v0, v1, v2);
        mat = Mat3::FromColumns(Normalize(v0), Normalize(v1), Normalize(v2));
    }

	namespace Mat3 {

		Matrix3 FromColumns(const M::Vector3& A1, const M::Vector3& A2, const M::Vector3& A3) {
			return Matrix3(A1.x, A2.x, A3.x,
						   A1.y, A2.y, A3.y,
						   A1.z, A2.z, A3.z);
		}

		Matrix3 Identity(void) {
			return Matrix3(1.0f, 0.0f, 0.0f,
						   0.0f, 1.0f, 0.0f,
						   0.0f, 0.0f, 1.0f);
		}

        Matrix3 Scale(float f) {
            return Matrix3(f, 0.0f, 0.0f,
                    0.0f, f, 0.0f,
                    0.0f, 0.0, f);
        }

        Matrix3 Scale(float fx, float fy, float fz) {
            return Matrix3(fx, 0.0f, 0.0f,
                           0.0f, fy, 0.0f,
                		   0.0f, 0.0f, fz);
        }

		Matrix3 RotateY(float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);

			return Matrix3(cosa, 0.0f, sina,
						   0.0f, 1.0f, 0.0f,
						   -sina, 0.0f, cosa);
		}

		Matrix3 RotateAxis(const Vector3& axis, float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);
			const float oneMinusCos = 1.0f - cosa;

			const float xyT = axis.x * axis.y * oneMinusCos;
			const float xzT = axis.x * axis.z * oneMinusCos;
			const float yzT = axis.y * axis.z * oneMinusCos;

			const float xT = axis.x * sina;
			const float yT = axis.y * sina;
			const float zT = axis.z * sina;

			return Matrix3(axis.x * axis.x * oneMinusCos + cosa, xyT - zT, xzT + yT,
						   xyT + zT, axis.y * axis.y * oneMinusCos + cosa, yzT - xT,
						   xzT - yT, yzT + xT, axis.z * axis.z * oneMinusCos + cosa);
		}

		Matrix3 RotateQuaternion(const Quaternion& q) {
			const float x2 = 2.0f * q.v.x * q.v.x;
			const float y2 = 2.0f * q.v.y * q.v.y;
			const float z2 = 2.0f * q.v.z * q.v.z;

			const float xy = 2.0f * q.v.x	* q.v.y;
			const float wz = 2.0f * q.w		* q.v.z;
			const float xz = 2.0f * q.v.x	* q.v.z;
			const float wy = 2.0f * q.w		* q.v.y;
			const float yz = 2.0f * q.v.y	* q.v.z;
			const float wx = 2.0f * q.w		* q.v.x;

			return Transpose(Matrix3(
				1.0f - y2 - z2, xy + wz, xz - wy,
				xy - wz, 1.0f - x2 - z2, yz + wx,
				xz + wy, yz - wx, 1.0f - x2 - y2
				));
		}

	} // namespace Mat3

} // namespace M
