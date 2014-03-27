#pragma once

#include <Nubuck\renderer\color\color.h>
#include <Nubuck\events\events.h>

BEGIN_EVENT_DEF(ENT_Geometry_EdgeRadiusChanged)
    float edgeRadius;
END_EVENT_DEF

BEGIN_EVENT_DEF(ENT_Geometry_EdgeColorChanged)
    R::Color edgeColor;
END_EVENT_DEF

BEGIN_EVENT_DEF(ENT_Geometry_TransparencyChanged)
    float transparency;
END_EVENT_DEF
    
BEGIN_EVENT_DEF(ENT_Geometry_RenderModeChanged)
    int renderMode;
END_EVENT_DEF

BEGIN_EVENT_DEF(ENT_Geometry_EdgeShadingChanged)
    IGeometry::ShadingMode::Enum shadingMode;
END_EVENT_DEF
