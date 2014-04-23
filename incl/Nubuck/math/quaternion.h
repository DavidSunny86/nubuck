#pragma once

#include "vector3.h"

namespace M {

    struct Matrix3;

	struct Quaternion {
		float	w;
		Vector3 v;

		Quaternion(void) { }
		Quaternion(const Quaternion& other);
		Quaternion(float w, const Vector3& v);

        Quaternion& operator=(const Quaternion& other);
        Quaternion  operator-() const;
	};

    Quaternion operator+(const Quaternion& lhp, const Quaternion& rhp);
    Quaternion operator*(float s, const Quaternion& q);
    Quaternion operator*(const Quaternion& q, float s);
	Quaternion operator*(const Quaternion& lhp, const Quaternion& rhp);

    float       Dot(const Quaternion& lhp, const Quaternion& rhp);
	Quaternion  Normalize(const Quaternion& q);
    Quaternion  Slerp(Quaternion q0, const Quaternion& q1, float t);

	namespace Quat {

		Quaternion Identity(void);
		Quaternion RotateAxis(const Vector3& axis, float angle);
        Quaternion FromMatrix(const Matrix3& m);

	} // namespace Quat

} // namespace M

#include "quaternion_mem_inl.h"
#include "quaternion_op_inl.h"