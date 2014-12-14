#pragma once

#include <Nubuck\renderer\color\color.h>
#include <Nubuck\events\events.h>

struct RenderModeEvent : EV::Event {
    EVENT_TYPE(RenderModeEvent)

    int     renderMode;
    bool    showWireframe;
    bool    showNormals;
};

struct EdgeShadingEvent : EV::Event {
    EVENT_TYPE(EdgeShadingEvent)

    int     shadingMode; // in Nubuck::ShadingMode
    bool    showHiddenLines;
};

extern EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_vertexScaleChanged;
extern EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_edgeScaleChanged;
extern EV::ConcreteEventDef<EV::Arg<R::Color> > ev_geom_edgeColorChanged;
extern EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_transparencyChanged;
extern EV::ConcreteEventDef<RenderModeEvent>    ev_geom_renderModeChanged;
extern EV::ConcreteEventDef<EdgeShadingEvent>   ev_geom_edgeShadingChanged;
