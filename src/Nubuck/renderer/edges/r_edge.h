#pragma once

#include <math\vector3.h>
#include <renderer\color\color.h>

namespace R {

struct Edge {
    M::Vector3  p0, p1;
    R::Color    color;
    float       radius;
};

} // namespace R