#pragma once

#include <Nubuck\nubuck.h>

struct Color {
    enum Enum {
        BLACK = 0,  // inner edges
        BLUE        // hull edges
    };
};

struct Globals {
    nb::geometry delaunay;
    nb::geometry chull;
    nb::geometry paraboloid;
    nb::geometry circle;

    leda::edge hullEdge; // of delaunay mesh

    // maps delaunay objects to chull objects
    leda::node_map<leda::node> vmap;
    leda::edge_map<leda::edge> emap;
};

Color::Enum GetColor(leda::nb::RatPolyMesh& mesh, leda::edge e);
void        SetColorU(leda::nb::RatPolyMesh& mesh, leda::edge e, Color::Enum color);

void ApplyEdgeColors(leda::nb::RatPolyMesh& mesh);