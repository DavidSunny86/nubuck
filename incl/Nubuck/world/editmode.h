#pragma once

namespace W {

struct editMode_t {
    enum Enum {
        OBJECTS     = 0,
        VERTICES,

        NUM_MODES,
        DEFAULT     = OBJECTS
    };
};

} // namespace W