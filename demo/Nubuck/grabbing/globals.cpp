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

/*
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
*/

template<typename IN_ETYPE, typename OUT_ETYPE>
void FromProjection(const leda::GRAPH<leda::rat_point, IN_ETYPE>& in, leda::GRAPH<point_t, OUT_ETYPE>& out, leda::edge_array<leda::edge>& emap) {
    leda::node_array<leda::node>  nmap(in, NULL);
    // leda::edge_array<leda::edge>  emap(in, NULL);
    emap.init(in, NULL);

    leda::node n;
    forall_nodes(n, in) {
        nmap[n] = out.new_node();
        out[nmap[n]] = point_t(in[n].xcoord(), in[n].ycoord(), 0);
    }


    leda::edge_map<bool> visited;
    visited.init(in, false);

    forall_nodes(n, in) {
        leda::list<leda::edge> L = in.out_edges(n);
        assert(!L.empty());
        leda::edge cur = NULL;
        leda::edge e;
        forall(e, L) {
            if(!cur) cur = out.new_edge(nmap[n], nmap[leda::target(e)]);
            else cur = out.new_edge(cur, nmap[leda::target(e)], 0, leda::behind);
            emap[e] = cur;
        }
    }

    visited.init(in, false);
    leda::edge e;
    forall_edges(e, in) {
        if(!visited[e]) {
            visited[e] = visited[in.reversal(e)] = true;
            out.set_reversal(emap[e], emap[in.reversal(e)]);
        }
    }
}

template<typename ETYPE>
static leda::node MinXY(const leda::GRAPH<leda::rat_point, ETYPE>& G, const leda::list<leda::node>& L) {
    leda::node min = L.front();
    for(int i = 1; i < L.size(); ++i) {
        leda::node other = L[L.get_item(i)];
        if(G[other].xcoord() < G[min].xcoord() || (G[other].xcoord() == G[min].xcoord() && G[other].ycoord() < G[min].ycoord()))
            min = other;
    }
    return min;
}

template<typename ETYPE>
static leda::edge FindOuterFace(const leda::GRAPH<leda::rat_point, ETYPE>& G) {
    leda::list<leda::node> L = G.all_nodes();
    leda::node n0 = MinXY(G, L);
    leda::edge e0 = NULL;
    leda::edge e;
    forall_out_edges(e, n0) {
        bool outerFace = true;
        leda::node n;
        forall_nodes(n, G) {
            if(0 < G.outdeg(n) && 0 < G.indeg(n) && 0 < leda::orientation(G[leda::source(e)], G[leda::target(e)], G[n]))
                outerFace = false;
        }
        if(outerFace) {
            e0 = e;
            break;
        }
    }
    assert(e0);
    return e0;
}

void Triangulate(const leda::GRAPH<leda::rat_point, leda::rat_segment>& in, leda::GRAPH<leda::rat_point, int>& out, leda::edge_map<leda::edge>& map) {
    assert(leda::Is_Planar_Map(in));
    assert(leda::Is_Plane_Map(in));

    leda::node_array<leda::node>  nmap(in, NULL);
    leda::edge_array<leda::edge>  emap(in, NULL);

    leda::node n;
    forall_nodes(n, in) {
        nmap[n] = out.new_node();
        out[nmap[n]] = in[n];
    }

    map.init(out, NULL);

    leda::edge_map<bool> visited;
    
    visited.init(in, false);

    /*
    leda::edge e;
    forall_edges(e, in) {
        if(!visited[e]) {
            leda::edge e0 = out.new_edge(nmap[leda::source(e)], nmap[leda::target(e)]);
            leda::edge e1 = out.new_edge(nmap[leda::target(e)], nmap[leda::source(e)]);
            out.set_reversal(e0, e1);
            map[e0] = e;
            map[e1] = in.reversal(e);
            emap[e] = e0;
            emap[in.reversal(e)] = e1;
            visited[e] = visited[in.reversal(e)] = true;
        }
    }
    */
    forall_nodes(n, in) {
        leda::list<leda::edge> L = in.out_edges(n);
        assert(!L.empty());
        leda::edge cur = NULL;
        leda::edge e;
        forall(e, L) {
            if(!cur) cur = out.new_edge(nmap[n], nmap[leda::target(e)]);
            else cur = out.new_edge(cur, nmap[leda::target(e)], 0, leda::behind);
            emap[e] = cur;
            map[cur] = e;
        }
    }

    visited.init(in, false);
    leda::edge e;
    forall_edges(e, in) {
        if(!visited[e]) {
            visited[e] = visited[in.reversal(e)] = true;
            out.set_reversal(emap[e], emap[in.reversal(e)]);
        }
    }

    assert(leda::Is_Bidirected(out));
    assert(leda::Is_Planar(out));
    assert(leda::Is_Planar_Map(out));
    assert(leda::Is_Plane_Map(out));


    visited.init(out, false);

    unsigned num = 0;
    e = FindOuterFace(out);
    leda::edge it = e;
    do {
        it = out.face_cycle_succ(it);
        visited[it] = true;
        num++;

        bool outerFace = true;
        forall_nodes(n, out) {
            if(0 < leda::orientation(out[leda::source(it)], out[leda::target(it)], out[n]))
                outerFace = false;
        }
        assert(outerFace);
    } while(e != it);
    assert(visited[e]);
    printf("FOUND NUM = %d\n", num);

    num = 0;
    forall_edges(e, out) {
        bool outerFace = true;
        forall_nodes(n, out) {
            if(0 < leda::left_turn(out[leda::source(e)], out[leda::target(e)], out[n]))
                outerFace = false;
        }
        if(outerFace && !visited[e]) num++;
    }
    printf("NOT FOUND NUM = %d\n", num);

    forall_edges(e, out) {
        if(!visited[e]) {
            leda::edge pred = out.face_cycle_pred(e);
            leda::edge it = out.face_cycle_succ(e);
            leda::edge cur = e;
            while(leda::source(pred) != leda::target(it)) {
                leda::edge next = out.face_cycle_succ(it);
                leda::edge e0 = out.new_edge(cur, leda::target(it), 0, leda::behind);
                leda::edge e1 = out.new_edge(out.reversal(it), leda::source(e), 0, leda::before);
                out.set_reversal(e0, e1);
                visited[e0] = visited[e1] = visited[it] = true;
                map[e0] = map[e1] = map[e];
                cur = e0;
                it = next;
            }
            visited[pred] = visited[e] = true;
        }
    }

    assert(leda::Is_Bidirected(out));
    assert(leda::Is_Planar(out));
    assert(leda::Is_Planar_Map(out));
    assert(leda::Is_Plane_Map(out));

    // printf("isTri = %d\n", leda::Is_Triangulation(out));
    assert(leda::Is_Triangulation(out));
}

void Delaunay2D(const graph_t& in, graph_t& out) {
    leda::list<point2_t> L(ToPointList2(in));
    graph2_t G2;
    leda::DELAUNAY_TRIANG(L, G2);
    out.clear();

    leda::edge_array<leda::edge> emap;
    FromProjection(G2, out, emap);
}

void ConvexHull(const graph_t& in, graph_t& out) {
    leda::list<point_t> L(ToPointList(in));
    out.clear();
    leda::CONVEX_HULL(L, out);
}

static void F(const leda::GRAPH<leda::rat_circle, point2_t>& VD, leda::GRAPH<leda::rat_point, int>& TRI, graph_t& G, leda::edge_map<leda::edge>& map) {
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
          leda::rat_point  cv  = cw + 10000 * inf * vec.rotate90();
          S.push_back(leda::segment(cw.to_point(),cv.to_point()));
        }
          
        else if (VD.outdeg(w) == 1) { //w at infinity
          leda::rat_point  cv  = VD[v].center();
          leda::rat_vector vec = VD[w].point3() - VD[w].point1();
          leda::rat_point  cw  = cv + 10000 * inf * vec.rotate90();
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

    leda::list<leda::node> del;
    leda::node n;
    forall_nodes(n, G2) {
        if(G2[n].xcoord() < -inf || G2[n].xcoord() > inf || G2[n].ycoord() < -inf || G2[n].ycoord() > inf)
            del.push_back(n);
    }
    forall(n, del) G2.del_node(n);

    TRI.clear();
    Triangulate(G2, TRI, map);

    // -----
    leda::edge_array<leda::edge> emap;
    FromProjection(G2, G, emap);
    
    forall_edges(e, TRI) {
        leda::edge inf = map[e];
        map[e] = emap[inf];
    }
}

void Voronoi2D(const graph_t& in, leda::GRAPH<leda::rat_point, int>& TRI, graph_t& out, leda::edge_map<leda::edge>& map) {
    leda::list<point2_t> L(ToPointList2(in));
    L.unique();
    leda::GRAPH<leda::rat_circle, point2_t> VD;
    leda::VORONOI(L, VD);
    TRI.clear();
    out.clear();
    F(VD, TRI, out, map);
}