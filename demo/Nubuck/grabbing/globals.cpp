#include "globals.h"
#include <LEDA\geo\geo_alg.h>

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
    leda::DELAUNAY_DIAGRAM(L, G2);
    out.clear();
    FromProjection(G2, out);
}