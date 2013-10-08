#pragma once

#include <Nubuck\nubuck.h>

typedef leda::rational              scalar_t;
typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   graph_t;

typedef leda::rat_point point2_t;
typedef leda::GRAPH<point2_t, int> graph2_t;

struct Globals {
    Nubuck  nb;

    leda::node_map<leda::node> nmap;

    bool showHull;

    graph_t         grNodes;
    graph_t         grHull;
    IPolyhedron*    phNodes;
    IPolyhedron*    phHull;

    graph_t         grNodesProj;
    graph_t 		grDelaunayProj;
    graph_t 		grHullProj;
    IPolyhedron*    phNodesProj;
    IPolyhedron* 	phDelaunayProj;
    IPolyhedron* 	phHullProj;
};

void Delaunay2D(const graph_t& in, graph_t& out);
void ConvexHull(const graph_t& in, graph_t& out);