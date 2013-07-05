#pragma once 

#include <common\types.h>
#include <system\locks\semaphore.h>

namespace W {

    enum {
		EVENT_APOCALYPSE,
        EVENT_SPAWN_ENTITY,
        EVENT_DESTROY_ENTITY,
        EVENT_CHANGE_COLOR,
        EVENT_UPDATE,   // new node positions, graph unchanged
        EVENT_REBUILD   // graph changed. eg. removed nodes, added edges
    };

    struct ChangeColorArgs {
        enum Mode {
            MODE_PULSE,
            MODE_LERP
        } mode;
        float       r, g, b;
        leda::edge  edge;
    };

    struct Event {
        int id; // in EventTypes
        
        // 0 is broadcast.
        // when EVENT_SPAWN_ENTITY == id, then entityId contains the id of
        // the new entity
        int entityId;

        SYS::Semaphore* sem;

        int type; // in EntityTypes, used by EVENT_SPAWN_ENTITY
        char args[512];
    };

} // namespace W