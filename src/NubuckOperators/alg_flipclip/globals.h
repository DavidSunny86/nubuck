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
    leda::list<leda::node>  L[2];
    Side::Enum              side;
    nb::geometry            geom[2];
    leda::edge              hullEdges[2];
    leda::node              stitchVerts[2];

    bool                    haltBeforeStitching;
};

enum {
    COLOR_MASK      = 3,
    PLANARITY_FLAG  = 4
};

inline void SetColorU(leda::nb::RatPolyMesh& mesh, leda::edge e, int color) {
    assert(0 <= color && color < 3);
    const leda::edge r = mesh.reversal(e);
    mesh[e] = mesh[e] & ~COLOR_MASK | color;
    mesh[r] = mesh[r] & ~COLOR_MASK | color;

    assert(Color::RED != color); // use InvalidateU for this
}

inline void InvalidateU(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    mesh[e] = mesh[mesh.reversal(e)] = Color::RED;
}

inline int GetColor(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    return mesh[e] & COLOR_MASK;
}

inline void MarkPlanar(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    mesh[e] |= PLANARITY_FLAG;
}

inline void ClearPlanarFlag(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    mesh[e] &= ~PLANARITY_FLAG;
}

inline bool IsMarkedPlanar(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    return mesh[e] & PLANARITY_FLAG;
}

void ApplyEdgeColors(leda::nb::RatPolyMesh& mesh);

struct EdgeInfo {
    bool isBlue;
    bool isConvex;
    bool isFlippable;
};

EdgeInfo GetEdgeInfo(leda::nb::RatPolyMesh& mesh, leda::edge e);