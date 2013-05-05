#pragma once 

#include <common\types.h>

namespace W {

    enum {
        EVENT_SPAWN_ENTITY,
        EVENT_CHANGE_COLOR,
        EVENT_REBUILD
    };

    struct ChangeColorArgs {
        enum Mode {
            MODE_PULSE,
            MODE_LERP
        } mode;
        float       r, g, b;
    };

    struct Event {
        int id; // in EventTypes
        
        // 0 is broadcast.
        // when EVENT_SPAWN_ENTITY == id, then entityId contains the id of
        // the new entity
        int entityId;

        int type; // in EntityTypes, used by EVENT_SPAWN_ENTITY
        char args[512];
    };

} // namespace W