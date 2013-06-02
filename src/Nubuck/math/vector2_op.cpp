#include "vector2.h"

namespace M {

    float Length(const Vector2& vector) {
        return sqrt(vector.x * vector.x + vector.y * vector.y);
    }

} // namespace M