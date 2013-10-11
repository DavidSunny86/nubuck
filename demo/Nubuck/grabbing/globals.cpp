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

static void F(const leda::GRAPH<leda::rat_circle, point2_t>& VD, graph_t& G) {
    leda::list<leda::rat_segment> S;

    scalar_t inf = 500;
    leda::edge_array<bool> drawn(VD, false);

    leda::edge e;
    forall_edges(e, VD) {
        if (drawn[e]) continue;

        drawn[VD.reversal(e)] = drawn[e] = true;

        leda::node v = source(e); leda::node w = target(e);

        if (VD.outdeg(v) == 1) { //v at infinity
          leda::rat_point  cw  = VD[w].center();
          leda::rat_vector vec = VD[v].point3() - VD[v].point1();
          leda::rat_point  cv  = cw + inf * vec.rotate90();
          S.push_back(leda::segment(cw.to_point(),cv.to_point()));
        }
          
        else if (VD.outdeg(w) == 1) { //w at infinity
          leda::rat_point  cv  = VD[v].center();
          leda::rat_vector vec = VD[w].point3() - VD[w].point1();
          leda::rat_point  cw  = cv + inf * vec.rotate90();
          S.push_back(leda::segment(cv.to_point(),cw.to_point()));
        }
           
        else  { //both v and w proper nodes
          leda::rat_point  cv  = VD[v].center();
          leda::rat_point  cw  = VD[w].center();
          S.push_back(leda::segment(cv.to_point(),cw.to_point()));
        }  
    }

    point2_t v0 = point2_t(-inf, -inf);
    point2_t v1 = point2_t( inf, -inf);
    point2_t v2 = point2_t( inf,  inf);
    point2_t v3 = point2_t(-inf,  inf);
    S.push_back(leda::rat_segment(v0, v1));
    S.push_back(leda::rat_segment(v1, v2));
    S.push_back(leda::rat_segment(v2, v3));
    S.push_back(leda::rat_segment(v3, v0));

    leda::GRAPH<leda::rat_point, leda::rat_segment> G2;
    // leda::SEGMENT_INTERSECTION(S, G2, true);
    leda::SWEEP_SEGMENTS(S, G2, true);

    leda::node_array<leda::node> nmap(G2, NULL);
    leda::node n;
    forall_nodes(n, G2) {
        leda::node n3 = nmap[n] = G.new_node();
        G[n3] = point_t(G2[n].xcoord(), G2[n].ycoord(), 0);
    }

    forall_edges(e, G2) {
        G.new_edge(nmap[leda::source(e)], nmap[leda::target(e)]);
    }

    leda::list<leda::node> del;
    forall_nodes(n, G) {
        if(G[n].xcoord() < -inf || G[n].xcoord() > inf || G[n].ycoord() < -inf || G[n].ycoord() > inf)
            del.push_back(n);
    }
    forall(n, del) G.del_node(n);

    G.make_planar_map();
}

void Voronoi2D(const graph_t& in, graph_t& out) {
    leda::list<point2_t> L(ToPointList2(in));
    leda::GRAPH<leda::rat_circle, point2_t> VD;
    leda::VORONOI(L, VD);
    out.clear();
    F(VD, out);
}