#pragma once

inline leda::d3_rat_point operator+(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
    return leda::d3_rat_point(
            lhp.xcoord() + rhp.xcoord(),
            lhp.ycoord() + rhp.ycoord(),
            lhp.zcoord() + rhp.zcoord());
}

inline leda::d3_rat_point operator*(const leda::d3_rat_point& vec, const leda::rational& s) {
    return leda::d3_rat_point(s * vec[0], s * vec[1], s * vec[2]);
}

inline leda::d3_rat_point operator*(const leda::rational& s, const leda::d3_rat_point& vec) {
    return vec * s;
}

