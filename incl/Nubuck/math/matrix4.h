#pragma once

namespace M {

	struct Vector3;
	struct Matrix3;
	struct Quaternion;

	struct Matrix4 {
		union {
			struct {
				float m00, m10, m20, m30;
				float m01, m11, m21, m31;
				float m02, m12, m22, m32;
				float m03, m13, m23, m33;
			};
			float mat[16];
		};

		Matrix4(void) { }
		Matrix4(float m00, float m01, float m02, float m03,
				float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23,
				float m30, float m31, float m32, float m33);
		Matrix4(const Matrix4& other);

		Matrix4& operator=(const Matrix4& other);
	};

	Matrix4 operator*(const Matrix4& lhp, const Matrix4& rhp);
    Matrix4 operator/(const Matrix4& mat, float scalar);

	Vector3 Transform(const Matrix4& mat, const Vector3& vec);
    
    float   Det(const Matrix4& m);
    Matrix4 Transpose(const Matrix4& mat);
    Matrix4 RigidInverse(const Matrix4& mat);
    bool    TryInvert(const Matrix4& m, Matrix4& inv);

    Vector3 TranslationOf(const Matrix4& mat);
    Matrix3 RotationOf(const Matrix4& mat);

	namespace Mat4 {

		Matrix4 ExpandedTR(const M::Matrix3& rot, const M::Vector3& tran);
		Matrix4 ExpandedTR(const M::Quaternion& rot, const M::Vector3& tran);

		Matrix4 Identity(void);
		Matrix4 Translate(float x, float y, float z);
		Matrix4 Translate(const M::Vector3& translation);
        Matrix4 Scale(float f);
        Matrix4 Scale(float fx, float fy, float fz);
		Matrix4 RotateX(float angle);
		Matrix4 RotateY(float angle);
		Matrix4 RotateZ(float angle);
        
		Matrix4 RotateAxis(const Vector3& axis, float angle);
        Matrix4 RotateMatrix(const Matrix3& rot);
        Matrix4 RotateQuaternion(const Quaternion& q);

        Matrix4 Perspective(float fovy, float aspect, float zNear, float zFar);
        Matrix4 Perspective(float fovy, float aspect, float zNear, float zFar, float zOff);
        Matrix4 Frustrum(float left, float right, float bottom, float top, float zNear, float zFar);
        Matrix4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);

	} // namespace Mat4

} // namespace M
