#pragma once

#include <Nubuck\nubuck.h>

struct Color {
    enum {
        BLACK = 0,  // convex
        RED,        // not convex
        BLUE        // hull edge
    };
};

// encapsulates data shared among all phases
struct Globals {
    Nubuck      nb;
    IGeometry*  geom;
    leda::edge  hullEdge;
};

void ApplyEdgeColors(leda::nb::RatPolyMesh& mesh);