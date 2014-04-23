#pragma once

#include "math.h"
#include "quaternion.h"

namespace M {

	M_INLINE Quaternion::Quaternion(const Quaternion& other) : w(other.w), v(other.v) { }

	M_INLINE Quaternion::Quaternion(float w, const Vector3& v) : w(w), v(v) { }

    M_INLINE Quaternion& Quaternion::operator=(const Quaternion& other) {
        if(&other != this) {
            w = other.w;
            v = other.v;
        }
        return *this;
    }

    M_INLINE Quaternion Quaternion::operator-() const {
        return Quaternion(-w, -v);
    }

} // namespace M