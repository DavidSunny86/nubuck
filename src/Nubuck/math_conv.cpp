#include <Nubuck\math_conv.h>

M::Vector3 ToVector(const leda::d3_rat_point& p) {
    /*
    sometimes this produces NaNs. no idea why

    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
    */
    return M::Vector3(
        static_cast<float>(p.xcoord().to_double()),
        static_cast<float>(p.ycoord().to_double()),
        static_cast<float>(p.zcoord().to_double()));
}

leda::d3_rat_point ToRatPoint(const M::Vector3& v) {
    return leda::d3_rat_point(leda::d3_point(v.x, v.y, v.z));
}