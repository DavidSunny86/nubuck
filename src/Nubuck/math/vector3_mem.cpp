#include "vector3.h"

namespace M {

	Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);

	float Vector3::Length(void) const {
		return sqrt(x * x + y * y + z * z);
	}

	void Vector3::Normalize(void) {
		const float len = Length();
		x /= len;
		y /= len;
		z /= len;
	}

    Vector3 Vector3::ProjectOn(const Vector3& other) {
        return (Dot(*this, other) / Dot(other, other)) * other;
    }

} // namespace M
