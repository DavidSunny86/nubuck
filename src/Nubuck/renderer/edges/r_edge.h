#pragma once

#include <Nubuck\renderer\color\color.h>
#include <math\vector3.h>

namespace R {

struct Edge {
    M::Vector3  p0, p1;
    R::Color    color;
    float       radius;
};

} // namespace R