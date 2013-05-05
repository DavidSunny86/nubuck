#pragma once

#include <math.h>
#include "math.h"
#include "quaternion.h"

namespace M {

	M_INLINE Quaternion operator*(const Quaternion& lhp, const Quaternion& rhp) {
		return Quaternion(
			lhp.w * rhp.w - Dot(lhp.v, rhp.v),
			lhp.w * rhp.v + rhp.w * lhp.v + Cross(rhp.v, lhp.v));
	}

	M_INLINE Quaternion Normalize(const Quaternion& q) {
		const float norm = sqrtf(q.w * q.w + Dot(q.v, q.v));
		const float oneOverNorm = 1.0f / norm;
		return Quaternion(oneOverNorm * q.w, oneOverNorm * q.v);
	}

	namespace Quat {

		M_INLINE Quaternion Identity(void) {
			return Quaternion(1.0f, Vector3::Zero);
		}

		M_INLINE Quaternion RotateAxis(const Vector3& axis, float angle) {
			const float hrad = 0.5f * Deg2Rad(angle);
			
			return Quaternion(cosf(hrad), sinf(hrad) * Normalize(axis));
		}

	} // namespace Quat

} // namespace M