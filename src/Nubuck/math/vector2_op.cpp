#include <Nubuck\math\vector2.h>

namespace M {

    float Dot(const Vector2& lhp, const Vector2& rhp) {
        return lhp.x * rhp.x + lhp.y * rhp.y;
    }

    float Length(const Vector2& vector) {
        return sqrt(vector.x * vector.x + vector.y * vector.y);
    }

    float Distance(const Vector2& lhp, const Vector2& rhp) {
        return Length(rhp - lhp);
    }

} // namespace M