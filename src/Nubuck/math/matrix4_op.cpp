#include "vector3.h"
#include "matrix3.h"
#include "quaternion.h"
#include "matrix4.h"

namespace M {

	Matrix4 operator*(const Matrix4& lhp, const Matrix4& rhp) {
		Matrix4 mat;
		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				float el = 0.0f;
				for(int k = 0; k < 4; ++k) {
					el += lhp.mat[i + 4 * k] * rhp.mat[k + 4 * j];
				}
				mat.mat[i + 4 * j] = el;
			}
		}
		return mat;
	}

	Vector3 Transform(const Matrix4& mat, const Vector3& vec) {
		Vector3 ret;
		for(int i = 0; i < 3; ++i) {
			float el = 0.0f;
			for(int j = 0; j < 3; ++j) {
				el += mat.mat[i + 4 * j] * vec.vec[j];
			}
			ret.vec[i] = el + mat.mat[i + 12];
		}
		return ret;
	}

    Matrix4 Transpose(const Matrix4& mat) {
        return Matrix4(
                mat.m00, mat.m10, mat.m20, mat.m30,
                mat.m01, mat.m11, mat.m21, mat.m31,
                mat.m02, mat.m12, mat.m22, mat.m32,
                mat.m03, mat.m13, mat.m23, mat.m33);
    }

    Matrix4 RigidInverse(const Matrix4& mat) {
        return Matrix4(
                mat.m00, mat.m10, mat.m20, -mat.m03,
                mat.m01, mat.m11, mat.m21, -mat.m13,
                mat.m02, mat.m12, mat.m22, -mat.m23,
                0.0f, 0.0f, 0.0f, 1.0f);
    }

    Vector3 TranslationOf(const Matrix4& mat) {
        return Vector3(mat.m03, mat.m13, mat.m23);
    }

    Matrix3 RotationOf(const Matrix4& mat) {
        return Matrix3(
                mat.m00, mat.m01, mat.m02,
                mat.m10, mat.m11, mat.m12,
                mat.m20, mat.m21, mat.m22);
    }

	namespace Mat4 {

		Matrix4 FromRigidTransform(const M::Matrix3& rot, const M::Vector3& tran) {
			return Matrix4(rot.m00, rot.m01, rot.m02, tran.x,
							rot.m10, rot.m11, rot.m12, tran.y,
							rot.m20, rot.m21, rot.m22, tran.z,
							0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 FromRigidTransform(const Quaternion& rot, const Vector3& tran) {
			return FromRigidTransform(Mat3::FromQuaternion(rot), tran);
		}

		Matrix4 FromTransform(const Quaternion& rot, const Vector3& tran, float scale) {
			return Scale(scale) * FromRigidTransform(rot, tran);
		}

		Matrix4 Identity(void) {
			return Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 Translate(float x, float y, float z) {
			return Matrix4(1.0f, 0.0f, 0.0f, x,
						   0.0f, 1.0f, 0.0f, y,
						   0.0f, 0.0f, 1.0f, z,
						   0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 Translate(const M::Vector3& translation) {
			return Translate(translation.x, translation.y, translation.z);
		}

        Matrix4 Scale(float f) {
            return Matrix4(f, 0.0f, 0.0f, 0.0f,
                    0.0f, f, 0.0f, 0.0f,
                    0.0f, 0.0, f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f);
        }

		Matrix4 RotateX(float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);

			return Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
						   0.0f, cosa, sina, 0.0f,
						   0.0f, -sina, cosa, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 RotateY(float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);

			return Matrix4(cosa, 0.0f, -sina, 0.0f,
						   0.0f, 1.0f, 0.0f, 0.0f,
						   sina, 0.0f, cosa, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 RotateZ(float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);

			return Matrix4(cosa, sina, 0.0f, 0.0f,
						   -sina, cosa, 0.0f, 0.0f,
						   0.0f, 0.0f, 1.0f, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 RotateAxis(const Vector3& axis, float angle) {
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

			return Matrix4(axis.x * axis.x * oneMinusCos + cosa, xyT - zT, xzT + yT, 0.0f,
						   xyT + zT, axis.y * axis.y * oneMinusCos + cosa, yzT - xT, 0.0f,
						   xzT - yT, yzT + xT, axis.z * axis.z * oneMinusCos + cosa, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
		}

        Matrix4 Perspective(float fovy, float aspect, float zNear, float zFar) {
            const float f = 1.0f / tan(0.5f * Deg2Rad(fovy));
            const float d = 1.0f / (zNear - zFar);

            return Matrix4(f / aspect, 0.0f, 0.0f, 0.0f,
                           0.0f, f, 0.0f, 0.0f,
                           0.0f, 0.0f, d * (zFar + zNear), d * (2 * zFar * zNear),
                           0.0f, 0.0f, -1.0f, 0.0f);
        }

        Matrix4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
            return Matrix4(2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
                    0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
                    0.0f, 0.0f, -2.0f / (zFar - zNear), -(zFar + zNear) / (zFar - zNear),
                    0.0f, 0.0f, 0.0f, 1.0f);
        }

	} // namespace Mat4

} // namespace M
