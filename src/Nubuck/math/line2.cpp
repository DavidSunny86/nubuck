#include "line2.h"

namespace M {

    Line2::Line2(const M::Vector2& p0, const M::Vector2& p1) 
        : a(p1.y - p0.y), b(p0.x - p1.x), c(a * p0.x + b * p0.y) { }

    Line2 Line2::FromPoints(const M::Vector2& p0, const M::Vector2& p1) {
        return Line2(p0, p1);
    }

    Line2 Line2::FromPointDirection(const M::Vector2& p, const M::Vector2& d) {
        return Line2(p, p + d);
    }

    // cnf. http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=geometry2#line_line_intersection
    bool Intersect(const Line2& lhp, const Line2& rhp, M::Vector2* where) {
        float det = lhp.a * rhp.b - rhp.a * lhp.b;
        //if(0.0f == det) return false;
        if(M::AlmostEqual(0.0f, det)) return false;
        if(where) {
            where->x = (rhp.b * lhp.c - lhp.b * rhp.c) / det;
            where->y = (lhp.a * rhp.c - rhp.a * lhp.c) / det;
        }
        return true;
    }

} // namespace M