#pragma once

#include <Nubuck\renderer\color\color.h>

#include <Nubuck\math\vector3.h>

namespace R {

    struct Light {
        M::Vector3  position;
        float       constantAttenuation;
        float       linearAttenuation;
        float       quadricAttenuation;
        Color       diffuseColor;
    };

} // namespace R