#pragma once

#include "vector3.h"

namespace M {

struct Plane {
    M::Vector3  n;
    float       d;

    static Plane FromPointNormal(const M::Vector3& p, const M::Vector3& n) {
        Plane pl;
        pl.n = Normalize(n);
        pl.d = -Dot(pl.n, p);
        return pl;
    }

    static Plane FromPointSpan(const M::Vector3& p, const M::Vector3& v0, const M::Vector3& v1) {
        return FromPointNormal(p, Normalize(Cross(v0, v1)));
    }
};

} // namespace M