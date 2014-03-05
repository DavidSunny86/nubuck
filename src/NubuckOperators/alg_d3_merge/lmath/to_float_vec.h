#pragma once

#include <LEDA/geo/d3_rat_point.h>

namespace LM {

    template<typename TYPE>
    struct ToFloatVec;

    template<>
    struct ToFloatVec<leda::d3_point> {
        static leda::vector Conv(const leda::d3_point& vec) {
            return vec.to_vector();
        }
    };

    template<>
    struct ToFloatVec<leda::d3_rat_point> {
        static leda::vector Conv(const leda::d3_rat_point& vec) {
            return leda::vector(vec[0].to_float(),
                    vec[1].to_float(),
                    vec[2].to_float());
        }
    };

} // namespace LM
