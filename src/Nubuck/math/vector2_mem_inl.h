#pragma once

#include "math.h"
#include "vector2.h"

namespace M {

	M_INLINE Vector2::Vector2(const Vector2& other) : x(other.x), y(other.y){ }

	M_INLINE Vector2::Vector2(float x, float y) : x(x), y(y) { }

	M_INLINE Vector2& Vector2::operator=(const Vector2& other) {
		if(&other == this) return *this;
		x = other.x;
		y = other.y;
		return *this;
	}

	M_INLINE Vector2 Vector2::operator-(void) const {
		return Vector2(-x, -y);
	}

	M_INLINE Vector2& Vector2::operator+=(const Vector2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	M_INLINE Vector2& Vector2::operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}

	M_INLINE Vector2& Vector2::operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		return *this;
	}

} // namespace M