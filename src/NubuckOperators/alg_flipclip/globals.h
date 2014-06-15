#pragma once

#include <Nubuck\polymesh.h>
#include <Nubuck\nubuck.h>

struct Color {
    enum {
        BLACK = 0,  // convex
        RED,        // not convex
        BLUE        // hull edge
    };
};

struct Side {
    enum Enum {
        FRONT = 0,
        BACK
    };
};

// encapsulates data shared among all phases
struct Globals {
    Nubuck                  nb;
    leda::list<leda::node>  L[2];
    Side::Enum              side;
    IGeometry*              geom[2];
    leda::edge              hullEdges[2];
    leda::node              stitchVerts[2];

    bool                    haltBeforeStitching;
};

inline void SetColor(leda::nb::RatPolyMesh& mesh, leda::edge e, int color) { mesh[e] = color; }
inline int  GetColor(leda::nb::RatPolyMesh& mesh, leda::edge e) { return mesh[e]; }

void ApplyEdgeColors(leda::nb::RatPolyMesh& mesh);