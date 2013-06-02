#pragma once

namespace M {

	struct Vector2 {
		union {
			struct { float x, y; };
			struct { float s, t; };
			float vec[2];
		};

		Vector2(void) { }
		Vector2(const Vector2& other);
		Vector2(float x, float y);

		Vector2&	operator=(const Vector2& other);
		Vector2		operator-(void) const;
		Vector2&	operator+=(const Vector2& other);
		Vector2&	operator*=(float scalar);
		Vector2&	operator/=(float scalar);

        static Vector2 Zero;
	};

	bool	operator==(const Vector2& lhp, const Vector2& rhp);
	Vector2 operator+(const Vector2& lhp, const Vector2& rhp);
	Vector2 operator-(const Vector2& lhp, const Vector2& rhp);
	Vector2 operator*(float scalar, const Vector2& vector);
	Vector2 operator*(const Vector2& vector, float scalar);
	Vector2 operator/(const Vector2& vector, float scalar);

    float   Length(const Vector2& vector);
    Vector2 Normalize(const Vector2& vector);

} // namespace M

#include "vector2_mem_inl.h"
#include "vector2_op_inl.h"