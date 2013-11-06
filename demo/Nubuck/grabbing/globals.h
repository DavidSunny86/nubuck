#pragma once

#include <Nubuck\nubuck.h>

typedef leda::rational              scalar_t;
typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   graph_t;

typedef leda::rat_point point2_t;
typedef leda::GRAPH<point2_t, int> graph2_t;

struct Color { float r, g, b; };

struct Globals {
    Nubuck  nb;

    leda::node_map<leda::node> nmap;

    bool showParaboloid;
    bool showHull;
    bool showVoronoi;
    bool showVoronoiEdges;

    leda::GRAPH<leda::rat_point, int> grVoronoiTri;
    leda::node_array<Color> colors;
    leda::edge_map<leda::edge> emap;

    IMesh*          paraboloid;
    IMesh*          decal;
    IMesh*          overlay;

    IPolyhedron*    phNodes;
    IPolyhedron*    phHull;

    IPolyhedron*    phNodesProj;
    IPolyhedron* 	phDelaunayProj;
    IPolyhedron*    phVoronoiProj;
    IPolyhedron* 	phHullProj;
};

void Triangulate(const graph_t& in, graph_t& out);
void Delaunay2D(const graph_t& in, graph_t& out);
void ConvexHull(const graph_t& in, graph_t& out);
void Voronoi2D(const graph_t& in, leda::GRAPH<leda::rat_point, int>& TRI, graph_t& out, leda::edge_map<leda::edge>& map);