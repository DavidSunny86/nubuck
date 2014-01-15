#pragma once

#include <Nubuck\renderer\color\color.h>

namespace R {

    struct Material {
        Color diffuseColor;

		static Material White;
    };

} // namespace R