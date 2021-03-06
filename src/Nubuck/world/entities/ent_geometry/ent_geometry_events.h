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

struct SetEntityVectorEvent : EV::Event {
    EVENT_TYPE(SetEntityVectorEvent)

    int             m_entityID;
    leda::rational  m_vector[4];
};

extern EV::ConcreteEventDef<EV::Arg<bool> >     ev_geom_showVertexLabels;
extern EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_setVertexLabelSize;
extern EV::ConcreteEventDef<EV::Arg<bool> >     ev_geom_xrayVertexLabels;
extern EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_vertexScaleChanged;
extern EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_edgeScaleChanged;
extern EV::ConcreteEventDef<EV::Arg<R::Color> > ev_geom_edgeTintChanged;
extern EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_transparencyChanged;
extern EV::ConcreteEventDef<RenderModeEvent>    ev_geom_renderModeChanged;
extern EV::ConcreteEventDef<EdgeShadingEvent>   ev_geom_edgeShadingChanged;

// user sets transformation of entity in outliner
extern EV::ConcreteEventDef<SetEntityVectorEvent>   ev_ent_usr_setPosition;
extern EV::ConcreteEventDef<SetEntityVectorEvent>   ev_ent_usr_setScale;

extern EV::ConcreteEventDef<SetEntityVectorEvent>   ev_ent_setPosition;
extern EV::ConcreteEventDef<SetEntityVectorEvent>   ev_ent_setScale;