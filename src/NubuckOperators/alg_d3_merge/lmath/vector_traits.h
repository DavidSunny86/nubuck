#pragma once

#include <LEDA/geo/d3_rat_point.h>

namespace LM {

    template<typename TYPE> struct VectorTraits;

    // d3_point
    template<>
    struct VectorTraits<leda::d3_point> {
        typedef float scalar_t;
    };

    // d3_rat_point
    template<>
    struct VectorTraits<leda::d3_rat_point> {
        typedef leda::rational scalar_t;
    };

} // namespace LM
