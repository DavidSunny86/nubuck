#pragma once

#include <common\types.h>
#include <renderer\renderer.h>
#include <world\world.h>
#include <world\entity.h>
#include <events\events.h>

BEGIN_EVENT_DEF(Apocalypse)
END_EVENT_DEF

BEGIN_EVENT_DEF(LinkEntity)
    W::Entity* entity;
END_EVENT_DEF

BEGIN_EVENT_DEF(SpawnPolyhedron)
    unsigned                entId;
    graph_t*    			G;
    leda::node_map<bool>*   cachedNodes;
    leda::edge_map<bool>*   cachedEdges;
END_EVENT_DEF

BEGIN_EVENT_DEF(SpawnMesh)
    unsigned                entId;
    R::meshPtr_t   meshPtr;
END_EVENT_DEF

BEGIN_EVENT_DEF(DestroyEntity)
    unsigned    entId;
END_EVENT_DEF

BEGIN_EVENT_DEF(Rebuild)
    unsigned                entId;
    graph_t*    			G;
    leda::node_map<bool>*   cachedNodes;
    leda::edge_map<bool>*   cachedEdges;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetVisible)
    unsigned    entId;
    bool        isVisible;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetName)
    unsigned    entId;
    const char* name;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetPosition)
    unsigned    entId;
    M::Vector3  pos;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetScale)
    unsigned    entId;
    float       sx, sy, sz;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetRotation)
    unsigned    entId;
    M::Matrix3  mat;
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

BEGIN_EVENT_DEF(SetEdgeColor)
    unsigned    entId;
    leda::edge  edge;
    R::Color    color;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetFaceColor)
    unsigned    entId;
    leda::edge  edge;
    R::Color    color;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetFaceVisibility)
    unsigned    entId;
    leda::edge  edge;
    bool        visible;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetHullAlpha)
    unsigned    entId;
    float       alpha;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetEdgeBaseColor)
    unsigned    entId;
    R::Color    color;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetEdgeRadius)
    unsigned    entId;
    float       radius;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetEffect)
    unsigned    entId;
    const char* fxName;
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