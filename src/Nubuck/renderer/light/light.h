#pragma once

#include <math\vector3.h>
#include <renderer\color\color.h>

namespace R {

    struct Light {
        M::Vector3  position;
        float       constantAttenuation;
        float       linearAttenuation;
        float       quadricAttenuation;
        Color       diffuseColor;
    };

} // namespace R