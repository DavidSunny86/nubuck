#pragma once

#include <math.h>
#include "math.h"
#include "vector2.h"

namespace M {

	M_INLINE bool operator==(const Vector2& lhp, const Vector2& rhp) {
		return lhp.x == rhp.x && lhp.y == rhp.y;
	}

	M_INLINE Vector2 operator+(const Vector2& lhp, const Vector2& rhp) {
		return Vector2(lhp.x + rhp.x, lhp.y + rhp.y);
	}

	M_INLINE Vector2 operator-(const Vector2& lhp, const Vector2& rhp) {
		return Vector2(lhp.x - rhp.x, lhp.y - rhp.y);
	}

	M_INLINE Vector2 operator*(float scalar, const Vector2& vector) {
		return Vector2(scalar * vector.x, scalar * vector.y);
	}

	M_INLINE Vector2 operator*(const Vector2& vector, float scalar) {
		return Vector2(scalar * vector.x, scalar * vector.y);
	}

	M_INLINE Vector2 operator/(const Vector2& vector, float scalar) {
		return Vector2(vector.x / scalar, vector.y / scalar);
	}

    M_INLINE Vector2 Normalize(const Vector2& vector) {
        float length = sqrt(vector.x * vector.x + vector.y * vector.y);
		return vector / length;
    }

} // namespace M