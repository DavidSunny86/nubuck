#pragma once

#include "vector3.h"

namespace M {

	struct Quaternion {
		float	w;
		Vector3 v;

		Quaternion(void) { }
		Quaternion(const Quaternion& other);
		Quaternion(float w, const Vector3& v);
	};

	Quaternion operator*(const Quaternion& lhp, const Quaternion& rhp);

	Quaternion Normalize(const Quaternion& q);

	namespace Quat {

		Quaternion Identity(void);
		Quaternion RotateAxis(const Vector3& axis, float angle);

	} // namespace Quat

} // namespace M

#include "quaternion_mem_inl.h"
#include "quaternion_op_inl.h"