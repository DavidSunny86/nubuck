#pragma once

#include <math.h>
#include "math.h"
#include "matrix3.h"
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

        // cnf. "3D Math Primer for Graphics and Game Dev.", Dunn/Parberry, p.189
        M_INLINE Quaternion FromMatrix(const Matrix3& m) {
            // transpose m since source uses row-major matrices
            const float m11 = m.m00;
            const float m12 = m.m10;
            const float m13 = m.m20;

            const float m21 = m.m01;
            const float m22 = m.m11;
            const float m23 = m.m21;

            const float m31 = m.m02;
            const float m32 = m.m12;
            const float m33 = m.m22;

            float w, x, y, z;

            float fourWSquaredMinus1 = m11 + m22 + m33;
            float fourXSquaredMinus1 = m11 - m22 - m33;
            float fourYSquaredMinus1 = m22 - m11 - m33;
            float fourZSquaredMinus1 = m33 - m11 - m22;

            int biggestIndex = 0;
            float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
            if(fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
                fourBiggestSquaredMinus1 = fourXSquaredMinus1;
                biggestIndex = 1;
            }
            if(fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
                fourBiggestSquaredMinus1 = fourYSquaredMinus1;
                biggestIndex = 2;
            }
            if(fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
                fourBiggestSquaredMinus1 = fourZSquaredMinus1;
                biggestIndex = 3;
            }

            float biggestVal = sqrt(fourBiggestSquaredMinus1 + 1.0f) * 0.5f;
            float mult = 0.25f / biggestVal;

            switch(biggestIndex) {
            case 0:
                w = biggestVal;
                x = (m23 - m32) * mult;
                y = (m31 - m13) * mult;
                z = (m12 - m21) * mult;
                break;

            case 1:
                x = biggestVal;
                w = (m23 - m32) * mult;
                y = (m12 + m21) * mult;
                z = (m31 + m13) * mult;
                break;

            case 2:
                y = biggestVal;
                w = (m31 - m13) * mult;
                x = (m12 + m21) * mult;
                z = (m23 + m32) * mult;
                break;

            case 3:
                z = biggestVal;
                w = (m12 - m21) * mult;
                x = (m31 + m13) * mult;
                y = (m23 + m32) * mult;
                break;
            }

            return Quaternion(w, Vector3(x, y, z));
        }

	} // namespace Quat

} // namespace M