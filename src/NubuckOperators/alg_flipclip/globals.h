#pragma once

#include <Nubuck\nubuck.h>

struct Color {
    enum {
        BLACK = 0,
        RED,
        BLUE
    };
};

// encapsulates data shared among all phases
struct Globals {
    Nubuck      nb;
    IGeometry*  geom;
};
