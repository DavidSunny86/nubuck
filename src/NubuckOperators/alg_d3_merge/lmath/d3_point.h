#pragma once

#include <LEDA/geo/d3_rat_point.h>
#include "lmath.h"

namespace leda {

    // leda headers dont't provide the prototypes,
    // impl is in _d3_rat_point.cpp though

    __declspec(dllimport) int orientation_xy(
        const d3_rat_point& a,
        const d3_rat_point& b,
        const d3_rat_point& c);

    __declspec(dllimport) int orientation(
        const d3_rat_point& a,
        const d3_rat_point& b,
        const d3_rat_point& c,
        const d3_rat_point& d);

} // namespace leda

leda::d3_rat_point operator+(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp);

leda::d3_rat_point operator*(const leda::d3_rat_point& vec, const leda::rational& s);
leda::d3_rat_point operator*(const leda::rational& s, const leda::d3_rat_point& vec);

namespace LM {

    int CmpLex(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp);

    leda::rational Dot(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp);

    leda::d3_rat_point Cross(const leda::d3_rat_point& lhp,
            const leda::d3_rat_point& rhp);

    leda::rational DotXY(const leda::d3_rat_point& lhp,
            const leda::d3_rat_point& rhp);

    int DistXY(const leda::d3_rat_point& p,
            const leda::d3_rat_point& q,  // -1
            const leda::d3_rat_point& r); // +1 

    int Dist(const leda::d3_rat_point& p,
            const leda::d3_rat_point& q,  // -1
            const leda::d3_rat_point& r); // +1 

    bool InHNeg(const leda::d3_rat_point& v0, 
            const leda::d3_rat_point& v1,
            const leda::d3_rat_point& v2,
            const leda::d3_rat_point& p);

} // namespace LM

#include "d3_point_inl.h"
