#pragma once

#include <Nubuck\events\events.h>

namespace W { class Entity; }

extern EV::ConcreteEventDef<EV::Event>              ev_w_apocalypse;
extern EV::ConcreteEventDef<EV::Arg<W::Entity*> >   ev_w_linkEntity;
extern EV::ConcreteEventDef<EV::Arg<unsigned> >     ev_w_destroyEntity;
extern EV::ConcreteEventDef<EV::Event>              ev_w_rebuildAll;
extern EV::ConcreteEventDef<EV::Event>              ev_w_selectionChanged;
extern EV::ConcreteEventDef<EV::Event>              ev_w_cameraChanged;
extern EV::ConcreteEventDef<EV::Arg<int> >          ev_w_editModeChanged;