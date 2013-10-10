#include "globals.h"
#include <LEDA\geo\geo_alg.h>
#include <LEDA\geo\d3_hull.h>

leda::list<point_t> ToPointList(const graph_t& G) {
    leda::list<point_t> L;
    leda::node n;
    forall_nodes(n, G) L.push_back(G[n]);
    return L;
}

leda::list<point2_t> ToPointList2(const graph_t& G) {
    leda::list<point2_t> L;
    leda::node n;
    forall_nodes(n, G) L.push_back(point2_t(G[n].xcoord(), G[n].ycoord()));
    return L;
}

void FromProjection(const graph2_t& G2, graph_t& G3) {
    leda::node_array<leda::node> nmap(G2, NULL);

    leda::node n2;
    forall_nodes(n2, G2) {
        leda::node n3 = nmap[n2] = G3.new_node();
        G3[n3] = point_t(G2[n2].xcoord(), G2[n2].ycoord(), 0);
    }

    leda::edge e2;
    forall_edges(e2, G2) {
        G3.new_edge(nmap[leda::source(e2)], nmap[leda::target(e2)]); 
    }
}

void Delaunay2D(const graph_t& in, graph_t& out) {
    leda::list<point2_t> L(ToPointList2(in));
    graph2_t G2;
    leda::DELAUNAY_TRIANG(L, G2);
    out.clear();
    FromProjection(G2, out);
}

void ConvexHull(const graph_t& in, graph_t& out) {
    leda::list<point_t> L(ToPointList(in));
    out.clear();
    leda::CONVEX_HULL(L, out);
}

void Voronoi2D(const graph_t& in, graph_t& out) {
    scalar_t inf = 10000;
    leda::list<point2_t> L(ToPointList2(in));
    L.push_back(point2_t( inf,  inf));
    L.push_back(point2_t( inf, -inf));
    L.push_back(point2_t(-inf,  inf));
    L.push_back(point2_t(-inf, -inf));
    leda::GRAPH<leda::rat_circle, point2_t> VD;
    leda::VORONOI(L, VD);
    out.clear();

    leda::node_array<leda::node> nmap(VD, NULL);

    leda::node n2;
    forall_nodes(n2, VD) {
        leda::node n3 = nmap[n2] = out.new_node();
        leda::rat_circle c = VD[n2];
        bool b = c.is_degenerate();
        if(!b) out[n3] = point_t(VD[n2].center().xcoord(), VD[n2].center().ycoord(), 0);
    }

    leda::edge e2;
    forall_edges(e2, VD) {
        if(VD.outdeg(leda::source(e2)) > 1 && VD.outdeg(leda::target(e2)) > 1)
            out.new_edge(nmap[leda::source(e2)], nmap[leda::target(e2)]); 
    }
    out.make_planar_map();
}