#pragma once

#include "vector2.h"

namespace M {

    struct Line2 {
    private:
        Line2(const M::Vector2& p0, const M::Vector2& p1);
    public:
        float a, b, c; // l: ax + by = c

        static Line2 FromPoints(const M::Vector2& p0, const M::Vector2& p1);
        static Line2 FromPointDirection(const M::Vector2& p, const M::Vector2& d);

        Line2(void) { }
    };

    // true = proper intersection, false = lines are parallel
    bool Intersect(const Line2& lhp, const Line2& rhp, M::Vector2* where = 0);

} // namespace M