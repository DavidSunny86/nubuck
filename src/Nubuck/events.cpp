#include <Nubuck\events\events.h>
#include <Nubuck\events\core_events.h>
#include <world\world_events.h>
#include <UI\outliner\outliner.h>
#include <operators\operator_events.h>
#include <world\entities\ent_geometry\ent_geometry_events.h>

eventID_t eventID_Cnt = 0;

eventID_t EV_GetNextID() {
    return ++eventID_Cnt;
}

// core events
NUBUCK_API EV::ConcreteEventDef<EV::ResizeEvent> ev_resize;
NUBUCK_API EV::ConcreteEventDef<EV::MouseEvent>  ev_mouse;
NUBUCK_API EV::ConcreteEventDef<EV::KeyEvent>    ev_key;

 // world events
EV::ConcreteEventDef<EV::Event>              ev_w_apocalypse;
EV::ConcreteEventDef<EV::Arg<W::Entity*> >   ev_w_linkEntity;
EV::ConcreteEventDef<EV::Arg<unsigned> >     ev_w_destroyEntity;
EV::ConcreteEventDef<EV::Event>              ev_w_rebuildAll;
EV::ConcreteEventDef<EV::Event>              ev_w_selectionChanged;
EV::ConcreteEventDef<EV::Event>              ev_w_cameraChanged;
EV::ConcreteEventDef<EV::Arg<int> >          ev_w_editModeChanged;

// outliner events
EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_createView;
EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_hide;
EV::ConcreteEventDef<EV::Args2<outlinerItem_t, QString*> >   ev_outl_setName;
EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_delete;

// operator events
EV::ConcreteEventDef<EV::Event>                  ev_op_actionFinished;
EV::ConcreteEventDef<EV::Arg<OP::Operator*> >    ev_op_setOperator;

// geometry events
EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_vertexScaleChanged;
EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_edgeScaleChanged;
EV::ConcreteEventDef<EV::Arg<R::Color> > ev_geom_edgeColorChanged;
EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_transparencyChanged;
EV::ConcreteEventDef<RenderModeEvent>    ev_geom_renderModeChanged;
EV::ConcreteEventDef<EdgeShadingEvent>   ev_geom_edgeShadingChanged;