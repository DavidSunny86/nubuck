#pragma once 

#include <common\types.h>
#include <renderer\color\color.h>

namespace W {

    enum EventType {
        EVENT_SPAWN_POLYHEDRON,
		EVENT_REBUILD,
        EVENT_SET_NODE_COLOR,
        EVENT_SET_FACE_COLOR,

        EVENT_RESIZE,
        EVENT_MOUSE
    };

    struct Event {
        int type; // in EventType
        char args[64];
    };

    struct EvArgs_SpawnPolyhedron {
        unsigned h;
        const graph_t* G;
    };

    struct EvArgs_Rebuild {
        unsigned entId;
    };

    struct EvArgs_SetNodeColor {
        unsigned entId;
        leda::node node;
        R::Color color;
    };

    struct EvArgs_SetFaceColor {
        unsigned entId;
        leda::edge edge;
        R::Color color;
    };

    struct EvArgs_Resize {
        int width, height;
    };

    struct EvArgs_Mouse {
        enum Type { MOUSE_DOWN, MOUSE_UP, MOUSE_WHEEL, MOUSE_MOVE };
        enum Button { 
            BUTTON_LEFT     = 1, // == Qt::LeftButton
            BUTTON_RIGHT    = 2, // == Qt::RightButton
            BUTTON_MIDDLE 
        };
        int type, button, delta, x, y;
    };

} // namespace W