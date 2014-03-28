#pragma once

#include <Nubuck\events\events.h>

namespace W { class Entity; }

BEGIN_EVENT_DEF(Apocalypse)
END_EVENT_DEF

BEGIN_EVENT_DEF(LinkEntity)
    W::Entity* entity;
END_EVENT_DEF

BEGIN_EVENT_DEF(DestroyEntity)
    unsigned    entId;
END_EVENT_DEF

BEGIN_EVENT_DEF(RebuildAll)
END_EVENT_DEF

BEGIN_EVENT_DEF(SelectionChanged)
END_EVENT_DEF

BEGIN_EVENT_DEF(CameraChanged)
END_EVENT_DEF

BEGIN_EVENT_DEF(EditModeChanged)
    int editMode;
END_EVENT_DEF