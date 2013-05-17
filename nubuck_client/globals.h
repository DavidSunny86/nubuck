#pragma once

#include <Nubuck\nubuck.h>

typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   graph_t;

struct Globals {
    Nubuck  nb;
    graph_t G;

    IPolyhedron* polyhedron;

    // vertices of the initial tetrahedron
    leda::node tVerts[4];

    // this is the list of nodes the algorithm successively
    // adds to the hull.
    // phase0 fills this list with every node except the
    // ones belonging to the initial tetrahedron
    leda::list<leda::node> L;
};