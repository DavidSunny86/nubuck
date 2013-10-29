#pragma once

#include <common\types.h>
#include <renderer\renderer.h>
#include <world\world.h>
#include <events\events.h>

BEGIN_EVENT_DEF(Apocalypse)
END_EVENT_DEF

BEGIN_EVENT_DEF(SpawnPolyhedron)
    unsigned    entId;
    graph_t*    G;
END_EVENT_DEF

BEGIN_EVENT_DEF(SpawnMesh)
    unsigned                entId;
    R::MeshMgr::meshPtr_t   meshPtr;
END_EVENT_DEF

BEGIN_EVENT_DEF(DestroyEntity)
    unsigned    entId;
END_EVENT_DEF

BEGIN_EVENT_DEF(Rebuild)
    unsigned    entId;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetVisible)
    unsigned    entId;
    bool        isVisible;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetName)
    unsigned    entId;
    const char* name;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetRenderFlags)
    unsigned    entId;
    int         flags;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetPickable)
    unsigned    entId;
    bool        isPickable;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetNodeColor)
    unsigned    entId;
    leda::node  node;
    R::Color    color;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetFaceColor)
    unsigned    entId;
    leda::edge  edge;
    R::Color    color;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetEdgeRadius)
    unsigned    entId;
    float       radius;
END_EVENT_DEF

BEGIN_EVENT_DEF(Resize)
    int         width;
    int         height;
END_EVENT_DEF

BEGIN_EVENT_DEF(Mouse)
    enum Type { MOUSE_DOWN, MOUSE_UP, MOUSE_WHEEL, MOUSE_MOVE };
    enum Button { 
        BUTTON_LEFT     = 1, // == Qt::LeftButton
        BUTTON_RIGHT    = 2, // == Qt::RightButton
        BUTTON_MIDDLE 
    };
    enum Modifier {
        MODIFIER_SHIFT = 0x02000000 // == Qt::ShiftModifier
    };
    int type, button, mods, delta, x, y;
END_EVENT_DEF

BEGIN_EVENT_DEF(Key)
    enum Type { KEY_DOWN = 0, KEY_UP };
    int type;
    int keyCode;
    bool autoRepeat;
END_EVENT_DEF

BEGIN_EVENT_DEF(RequestEntityInfo)
    unsigned entId;
END_EVENT_DEF

BEGIN_EVENT_DEF(EntityInfo)
    int             entType; // in World::EntityType
    W::EntityInf*   inf;
END_EVENT_DEF