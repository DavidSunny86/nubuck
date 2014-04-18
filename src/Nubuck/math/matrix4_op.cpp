#include <assert.h>
#include <Nubuck\common\common.h>
#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix3.h>
#include <Nubuck\math\quaternion.h>
#include <Nubuck\math\matrix4.h>

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
    
    Matrix4 operator/(const Matrix4& mat, float scalar) {
        const float f = 1.0f / scalar;
        return Matrix4(
            f * mat.m00, f * mat.m01, f * mat.m02, f * mat.m03,
            f * mat.m10, f * mat.m11, f * mat.m12, f * mat.m13,
            f * mat.m20, f * mat.m21, f * mat.m22, f * mat.m23,
            f * mat.m30, f * mat.m31, f * mat.m32, f * mat.m33);
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

    float Det(const Matrix4& m) {
        return
              m.m00 * m.m11 * m.m22 * m.m33 + m.m00 * m.m12 * m.m23 * m.m31 + m.m00 * m.m13 * m.m21 * m.m32
            + m.m01 * m.m10 * m.m23 * m.m32 + m.m01 * m.m12 * m.m20 * m.m33 + m.m01 * m.m13 * m.m22 * m.m30
            + m.m02 * m.m10 * m.m21 * m.m33 + m.m02 * m.m11 * m.m23 * m.m30 + m.m02 * m.m13 * m.m20 * m.m31
            + m.m03 * m.m10 * m.m22 * m.m31 + m.m03 * m.m11 * m.m20 * m.m32 + m.m03 * m.m12 * m.m21 * m.m30
            - m.m00 * m.m11 * m.m23 * m.m32 - m.m00 * m.m12 * m.m21 * m.m33 - m.m00 * m.m13 * m.m22 * m.m31
            - m.m01 * m.m10 * m.m22 * m.m33 - m.m01 * m.m12 * m.m23 * m.m30 - m.m01 * m.m13 * m.m20 * m.m32
            - m.m02 * m.m10 * m.m23 * m.m31 - m.m02 * m.m11 * m.m20 * m.m33 - m.m02 * m.m13 * m.m21 * m.m30
            - m.m03 * m.m10 * m.m21 * m.m32 - m.m03 * m.m11 * m.m22 * m.m30 - m.m03 * m.m12 * m.m20 * m.m31;
    }

    Matrix4 Transpose(const Matrix4& mat) {
        return Matrix4(
                mat.m00, mat.m10, mat.m20, mat.m30,
                mat.m01, mat.m11, mat.m21, mat.m31,
                mat.m02, mat.m12, mat.m22, mat.m32,
                mat.m03, mat.m13, mat.m23, mat.m33);
    }

    // cnf 'http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix'
    bool TryInvert(const Matrix4& m, Matrix4& inv) {
        inv.mat[0] = 
            m.mat[5]  * m.mat[10] * m.mat[15] - 
            m.mat[5]  * m.mat[11] * m.mat[14] - 
            m.mat[9]  * m.mat[6]  * m.mat[15] + 
            m.mat[9]  * m.mat[7]  * m.mat[14] +
            m.mat[13] * m.mat[6]  * m.mat[11] - 
            m.mat[13] * m.mat[7]  * m.mat[10];

        inv.mat[4] = 
            -m.mat[4]  * m.mat[10] * m.mat[15] + 
             m.mat[4]  * m.mat[11] * m.mat[14] + 
             m.mat[8]  * m.mat[6]  * m.mat[15] - 
             m.mat[8]  * m.mat[7]  * m.mat[14] - 
             m.mat[12] * m.mat[6]  * m.mat[11] + 
             m.mat[12] * m.mat[7]  * m.mat[10];

        inv.mat[8] = 
            m.mat[4]  * m.mat[9]  * m.mat[15] - 
            m.mat[4]  * m.mat[11] * m.mat[13] - 
            m.mat[8]  * m.mat[5]  * m.mat[15] + 
            m.mat[8]  * m.mat[7]  * m.mat[13] + 
            m.mat[12] * m.mat[5]  * m.mat[11] - 
            m.mat[12] * m.mat[7]  * m.mat[9];

        inv.mat[12] = 
            -m.mat[4]  * m.mat[9]  * m.mat[14] + 
             m.mat[4]  * m.mat[10] * m.mat[13] +
             m.mat[8]  * m.mat[5]  * m.mat[14] - 
             m.mat[8]  * m.mat[6]  * m.mat[13] - 
             m.mat[12] * m.mat[5]  * m.mat[10] + 
             m.mat[12] * m.mat[6]  * m.mat[9];

        inv.mat[1] = 
            -m.mat[1]  * m.mat[10] * m.mat[15] + 
             m.mat[1]  * m.mat[11] * m.mat[14] + 
             m.mat[9]  * m.mat[2]  * m.mat[15] - 
             m.mat[9]  * m.mat[3]  * m.mat[14] - 
             m.mat[13] * m.mat[2]  * m.mat[11] + 
             m.mat[13] * m.mat[3]  * m.mat[10];

        inv.mat[5] = 
            m.mat[0]  * m.mat[10] * m.mat[15] - 
            m.mat[0]  * m.mat[11] * m.mat[14] - 
            m.mat[8]  * m.mat[2]  * m.mat[15] + 
            m.mat[8]  * m.mat[3]  * m.mat[14] + 
            m.mat[12] * m.mat[2]  * m.mat[11] - 
            m.mat[12] * m.mat[3]  * m.mat[10];

        inv.mat[9] = 
            -m.mat[0]  * m.mat[9]  * m.mat[15] + 
             m.mat[0]  * m.mat[11] * m.mat[13] + 
             m.mat[8]  * m.mat[1]  * m.mat[15] - 
             m.mat[8]  * m.mat[3]  * m.mat[13] - 
             m.mat[12] * m.mat[1]  * m.mat[11] + 
             m.mat[12] * m.mat[3]  * m.mat[9];

        inv.mat[13] = 
            m.mat[0]  * m.mat[9]  * m.mat[14] - 
            m.mat[0]  * m.mat[10] * m.mat[13] - 
            m.mat[8]  * m.mat[1]  * m.mat[14] + 
            m.mat[8]  * m.mat[2]  * m.mat[13] + 
            m.mat[12] * m.mat[1]  * m.mat[10] - 
            m.mat[12] * m.mat[2]  * m.mat[9];

        inv.mat[2] = 
            m.mat[1]  * m.mat[6] * m.mat[15] - 
            m.mat[1]  * m.mat[7] * m.mat[14] - 
            m.mat[5]  * m.mat[2] * m.mat[15] + 
            m.mat[5]  * m.mat[3] * m.mat[14] + 
            m.mat[13] * m.mat[2] * m.mat[7] - 
            m.mat[13] * m.mat[3] * m.mat[6];

        inv.mat[6] = 
            -m.mat[0]  * m.mat[6] * m.mat[15] + 
             m.mat[0]  * m.mat[7] * m.mat[14] + 
             m.mat[4]  * m.mat[2] * m.mat[15] - 
             m.mat[4]  * m.mat[3] * m.mat[14] - 
             m.mat[12] * m.mat[2] * m.mat[7] + 
             m.mat[12] * m.mat[3] * m.mat[6];

        inv.mat[10] = 
            m.mat[0]  * m.mat[5] * m.mat[15] - 
            m.mat[0]  * m.mat[7] * m.mat[13] - 
            m.mat[4]  * m.mat[1] * m.mat[15] + 
            m.mat[4]  * m.mat[3] * m.mat[13] + 
            m.mat[12] * m.mat[1] * m.mat[7] - 
            m.mat[12] * m.mat[3] * m.mat[5];

        inv.mat[14] = 
            -m.mat[0]  * m.mat[5] * m.mat[14] + 
             m.mat[0]  * m.mat[6] * m.mat[13] + 
             m.mat[4]  * m.mat[1] * m.mat[14] - 
             m.mat[4]  * m.mat[2] * m.mat[13] - 
             m.mat[12] * m.mat[1] * m.mat[6] + 
             m.mat[12] * m.mat[2] * m.mat[5];

        inv.mat[3] = 
            -m.mat[1] * m.mat[6] * m.mat[11] + 
             m.mat[1] * m.mat[7] * m.mat[10] + 
             m.mat[5] * m.mat[2] * m.mat[11] - 
             m.mat[5] * m.mat[3] * m.mat[10] - 
             m.mat[9] * m.mat[2] * m.mat[7] + 
             m.mat[9] * m.mat[3] * m.mat[6];

        inv.mat[7] = 
            m.mat[0] * m.mat[6] * m.mat[11] - 
            m.mat[0] * m.mat[7] * m.mat[10] - 
            m.mat[4] * m.mat[2] * m.mat[11] + 
            m.mat[4] * m.mat[3] * m.mat[10] + 
            m.mat[8] * m.mat[2] * m.mat[7] - 
            m.mat[8] * m.mat[3] * m.mat[6];

        inv.mat[11] = 
            -m.mat[0] * m.mat[5] * m.mat[11] + 
             m.mat[0] * m.mat[7] * m.mat[9] + 
             m.mat[4] * m.mat[1] * m.mat[11] - 
             m.mat[4] * m.mat[3] * m.mat[9] - 
             m.mat[8] * m.mat[1] * m.mat[7] + 
             m.mat[8] * m.mat[3] * m.mat[5];

        inv.mat[15] = 
            m.mat[0] * m.mat[5] * m.mat[10] - 
            m.mat[0] * m.mat[6] * m.mat[9] - 
            m.mat[4] * m.mat[1] * m.mat[10] + 
            m.mat[4] * m.mat[2] * m.mat[9] + 
            m.mat[8] * m.mat[1] * m.mat[6] - 
            m.mat[8] * m.mat[2] * m.mat[5];

        float det = m.mat[0] * inv.mat[0] + m.mat[1] * inv.mat[4] + m.mat[2] * inv.mat[8] + m.mat[3] * inv.mat[12];

        if (det == 0) return false;

        const float oneOverDet = 1.0f / det;
        for (int i = 0; i < 16; i++) inv.mat[i] = inv.mat[i] * oneOverDet;

#ifdef _DEBUG
        M::Matrix4 id0 = M::Mat4::Identity();
        M::Matrix4 id1 = inv * m;
        for(int i = 0; i < 16; ++i) {
            if(!M::AlmostEqual(id0.mat[i], id1.mat[i])) {
                common.printf("WARNING - A = M * inv(M) != I, A.mat[%d] = %f.\n", i, id1.mat[i]);
            }
        }
#endif

        return true;
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

        Matrix4 Scale(float fx, float fy, float fz) {
            return Matrix4(fx, 0.0f, 0.0f, 0.0f,
                    0.0f, fy, 0.0f, 0.0f,
                    0.0f, 0.0, fz, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f);
        }

		Matrix4 RotateX(float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);

			return Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
						   0.0f, cosa, -sina, 0.0f,
						   0.0f, sina, cosa, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 RotateY(float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);

			return Matrix4(cosa, 0.0f, sina, 0.0f,
						   0.0f, 1.0f, 0.0f, 0.0f,
						   -sina, 0.0f, cosa, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4 RotateZ(float angle) {
			const float rad = Deg2Rad(angle);
			const float sina = sinf(rad);
			const float cosa = cosf(rad);

			return Matrix4(cosa, -sina, 0.0f, 0.0f,
						   sina, cosa, 0.0f, 0.0f,
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

        Matrix4 Perspective(float fovy, float aspect, float zNear, float zFar, float zOff) {
            const float f = 1.0f / tan(0.5f * Deg2Rad(fovy));
            const float d = 1.0f / (zNear - zFar);

            return Matrix4(f / aspect, 0.0f, 0.0f, 0.0f,
                           0.0f, f, 0.0f, 0.0f,
                           0.0f, 0.0f, (1.0f + zOff) * d * (zFar + zNear), d * (2 * zFar * zNear),
                           0.0f, 0.0f, -1.0f, 0.0f);
        }

        Matrix4 Frustrum(float left, float right, float bottom, float top, float zNear, float zFar) {
            return Matrix4((2.0f * zNear) / (right - left), 0.0f, (right + left) / (right - left), 0.0f,
                0.0f, (2.0f * zNear) / (top - bottom), (top + bottom) / (top - bottom), 0.0f,
                0.0f, 0.0f, -(zFar + zNear) / (zFar - zNear), -(2.0f * zNear * zFar) / (zFar - zNear),
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
