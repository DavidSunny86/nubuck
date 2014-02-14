#pragma once

#include <Nubuck\renderer\color\color.h>
#include <events\events.h>

BEGIN_EVENT_DEF(ENT_Geometry_EdgeRadiusChanged)
    float edgeRadius;
END_EVENT_DEF

BEGIN_EVENT_DEF(ENT_Geometry_EdgeColorChanged)
    R::Color edgeColor;
END_EVENT_DEF

