#pragma once

#include <Nubuck\nubuck.h>

typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   graph_t;

struct Globals {
    Nubuck  nb;
    graph_t G;

    IPolyhedron* _delaunay;
};