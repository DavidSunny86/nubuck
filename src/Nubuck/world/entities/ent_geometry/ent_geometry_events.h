#pragma once

#include <Nubuck\renderer\color\color.h>
#include <Nubuck\events\events.h>

BEGIN_EVENT_DEF(ENT_Geometry_VertexScaleChanged)
    float vertexScale;
END_EVENT_DEF

BEGIN_EVENT_DEF(ENT_Geometry_EdgeScaleChanged)
    float edgeScale;
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
    Nubuck::ShadingMode::Enum    shadingMode;
    bool                            showHiddenLines;
END_EVENT_DEF

