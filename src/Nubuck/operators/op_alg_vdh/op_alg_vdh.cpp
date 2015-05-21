#include <maxint.h>

#include <LEDA\geo\geo_alg.h>
#include <LEDA\geo\d3_hull.h>
#include <Nubuck\animation\move_vertex_anim.h>
#include "op_alg_vdh.h"

using namespace leda;
using namespace NB;

typedef leda::rat_point                                 point2_t;
typedef leda::GRAPH<point2_t, int>                      graph2_t;
typedef leda::GRAPH<leda::rat_circle, leda::rat_point>  voronoiGraph_t;

namespace OP {

/*
==================================================
    VDH_Panel implementation
==================================================
*/

VDH_Panel::VDH_Panel() {
    NB::BoxLayout boxLayout = NB::CreateVerticalBoxLayout();

    _cbDelaunay = NB::CreateCheckBox(ID_DELAUNAY, "Show Delaunay");
    _cbVoronoi = NB::CreateCheckBox(ID_VORONOI, "Show Voronoi");
    _cbCHull = NB::CreateCheckBox(ID_CHULL, "Show Hull");
    _cbParaboloid = NB::CreateCheckBox(ID_PARABOLOID, "Show Paraboloid");

    NB::AddWidgetToBox(boxLayout, NB::CastToWidget(_cbDelaunay));
    NB::AddWidgetToBox(boxLayout, NB::CastToWidget(_cbVoronoi));
    NB::AddWidgetToBox(boxLayout, NB::CastToWidget(_cbCHull));
    NB::AddWidgetToBox(boxLayout, NB::CastToWidget(_cbParaboloid));

    SetLayout(boxLayout);
}

void VDH_Panel::Invoke() {
    NB::SetChecked(_cbDelaunay, true, true);
    NB::SetChecked(_cbVoronoi, true, true);
    NB::SetChecked(_cbCHull, true, true);
    NB::SetChecked(_cbParaboloid, true, true);
}

/*
==================================================
    VDH_Operator implementation
==================================================
*/

static leda::list<point2_t> ToPointList2(const NB::Graph& G) {
    leda::list<point2_t> L;
    leda::node n;
    forall_nodes(n, G) L.push_back(point2_t(G[n].xcoord(), G[n].ycoord()));
    return L;
}

template<typename IN_VDATA, typename IN_EDATA>
static leda::d3_rat_point EmbeddedPosition(const leda::GRAPH<IN_VDATA, IN_EDATA>& in, const leda::node v) {
    return leda::d3_rat_point(in[v].xcoord(), in[v].ycoord(), 0);
}

static leda::d3_rat_point EmbeddedPosition(const voronoiGraph_t& VD, const leda::node v) {
    COM_assert(!VD[v].is_line());
    const point2_t c = VD[v].center();
    return leda::d3_rat_point(c.xcoord(), c.ycoord(), 0);
}

template<typename IN_VDATA, typename IN_EDATA>
static void FromProjection(
    const leda::GRAPH<IN_VDATA, IN_EDATA>& in,
    leda::GRAPH<leda::d3_rat_point, int>& out,
    leda::edge_array<leda::edge>& emap)
{
    leda::node_array<leda::node>  nmap(in, NULL);
    emap.init(in, NULL);

    leda::node n;
    forall_nodes(n, in) {
        nmap[n] = out.new_node();
        out[nmap[n]] = EmbeddedPosition(in, n);
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

void Delaunay2D(const NB::Graph& in, NB::Graph& out) {
    leda::list<point2_t> L(ToPointList2(in));
    graph2_t G2;
    leda::DELAUNAY_TRIANG(L, G2);
    out.clear();

    leda::edge_array<leda::edge> emap;
    FromProjection(G2, out, emap);
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

static void Triangulate(const leda::GRAPH<leda::rat_point, leda::rat_segment>& in, leda::GRAPH<leda::rat_point, int>& out, leda::edge_map<leda::edge>& map) {
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
            if(leda::left_turn(out[leda::source(e)], out[leda::target(e)], out[n]))
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

static const leda::rational inf = 10;

static void CreateBoundedSegmentation(
    const leda::GRAPH<leda::rat_circle, point2_t>& VD, 
    leda::GRAPH<leda::rat_point, leda::rat_segment>& G2)
{
    leda::list<leda::rat_segment> S0;
    leda::list<leda::rat_segment> S1;

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
          S0.push_back(leda::segment(cw.to_point(),cv.to_point()));
        }
          
        else if (VD.outdeg(w) == 1) { //w at infinity
          leda::rat_point  cv  = VD[v].center();
          leda::rat_vector vec = VD[w].point3() - VD[w].point1();
          leda::rat_point  cw  = cv + 10000 * inf * vec.rotate90();
          S0.push_back(leda::segment(cv.to_point(),cw.to_point()));
        }
           
        else  { //both v and w proper nodes
          leda::rat_point  cv  = VD[v].center();
          leda::rat_point  cw  = VD[w].center();
          S0.push_back(leda::segment(cv.to_point(),cw.to_point()));
        }  
    }

    point2_t v0 = point2_t(-inf, -inf);
    point2_t v1 = point2_t( inf, -inf);
    point2_t v2 = point2_t( inf,  inf);
    point2_t v3 = point2_t(-inf,  inf);
    S1.push_back(leda::rat_segment(v0, v1));
    S1.push_back(leda::rat_segment(v1, v2));
    S1.push_back(leda::rat_segment(v2, v3));
    S1.push_back(leda::rat_segment(v3, v0));

    G2.clear();
    leda::SEGMENT_INTERSECTION(S0, S1, G2, true);

    leda::list<leda::node> del;
    leda::node n;
    forall_nodes(n, G2) {
        if(G2[n].xcoord() < -inf || G2[n].xcoord() > inf || G2[n].ycoord() < -inf || G2[n].ycoord() > inf)
            del.push_back(n);
    }
    forall(n, del) G2.del_node(n);
}

static void Voronoi2D(const NB::Graph& in, NB::Graph& out) {
    leda::edge_map<leda::edge> map;

    leda::list<point2_t> L(ToPointList2(in));
    L.unique();

    leda::GRAPH<leda::rat_circle, point2_t> VD;
    leda::VORONOI(L, VD);

    leda::GRAPH<leda::rat_point, leda::rat_segment> G2;
    CreateBoundedSegmentation(VD, G2);

    leda::GRAPH<leda::rat_point, int> TRI;
    Triangulate(G2, TRI, map);

    leda::edge_array<leda::edge> emap;
    out.clear();
    FromProjection(G2, out, emap);
    
    leda::edge e;
    forall_edges(e, TRI) {
        leda::edge inf = map[e];
        map[e] = emap[inf];
    }
}

static leda::edge NextDanglingEdge(const voronoiGraph_t& VD, const leda::edge e, const leda::edge de0) {
    leda::edge it = VD.face_cycle_succ(e);
    while(de0 != it && 1 != VD.outdeg(VD.target(it))) {
        it = VD.face_cycle_succ(it);
    }
    return it;
}

static leda::node FindVertexByPosition2(const NB::Graph& G, const leda::rat_point& p) {
    leda::node v;
    forall_nodes(v, G) {
        if(p == G[v].project_xy()) return v;
    }
    return NULL;
}

/*
site[e] is vertex of graph 'in' that lies left of e or NULL, but for each face
there is at least one edge with site != NULL.
TODO:
-seg-box intersection of segments with no interior endpoints
-special cases: interior contains no segments, no interior points
*/
static leda::edge Voronoi2D_BruteForce(const NB::Graph& in, NB::Graph& out, leda::edge_map<leda::node>& site) {
    typedef leda::rat_segment   seg_t;
    typedef leda::rat_ray       ray_t;
    typedef leda::rat_circle    circle_t;

    const point2_t boxVerts[] = {
        point2_t(-inf, -inf),
        point2_t(-inf,  inf),
        point2_t( inf,  inf),
        point2_t( inf, -inf)
    };

    const leda::rat_segment boxSegs[] = {
        seg_t(boxVerts[0], boxVerts[1]),
        seg_t(boxVerts[1], boxVerts[2]),
        seg_t(boxVerts[2], boxVerts[3]),
        seg_t(boxVerts[3], boxVerts[0])
    };

    leda::rat_rectangle box(boxVerts[0], boxVerts[2]);

    leda::list<point2_t> L(ToPointList2(in));
    L.unique();

    voronoiGraph_t VD;
    leda::VORONOI(L, VD);

    leda::node_array<bool> inBox(VD, false);
    leda::list<leda::node> outside;
    leda::edge_array<bool> visited(VD, false);

    leda::node v;
    leda::node w;

    forall_nodes(v, VD) {
        if(!VD[v].is_line() && box.inside_or_contains(VD[v].center())) {
            inBox[v] = true;
        } else outside.push(v);
    }

    leda::edge de0 = NULL;

    unsigned numEdges = VD.number_of_edges();

    leda::edge e = VD.first_edge();
    unsigned i = 0;
    while(i < numEdges) {
        v = VD.source(e);
        w = VD.target(e);
        if(!visited[e] && inBox[v] && !inBox[w]) {
            // edge intersects box, clip

            if(VD[w].is_line()) {
                // w at infinity, intersect box with ray.
                // NOTE that we cannot test 1 == outdeg(v),
                // because we move edges.
                // However, rat_circle::is_line() is cheap.

                leda::rat_vector perp = VD[w].point3() - VD[w].point1();
                leda::rat_ray ray(VD[v].center(), perp.rotate90());

                leda::rat_point isect;
                bool isIntersecting = false;
                for(int i = 0; i < 4; ++i) {
                    if(ray.intersection(boxSegs[i], isect)) {
                        isIntersecting = true;
                        break;
                    }
                }
                assert(isIntersecting);

                VD[w] = leda::rat_circle(isect);
                inBox[w] = true;

                // an unbounded edge is part of the back face
                de0 = e;
            } else {
                // w is proper node, intersect box with segment

                leda::rat_segment seg(VD[v].center(), VD[w].center());

                leda::rat_point isect;
                bool isIntersecting = false;
                for(int i = 0; i < 4; ++i) {
                    if(seg.intersection(boxSegs[i], isect)) {
                        isIntersecting = true;
                        break;
                    }
                }
                assert(isIntersecting);

                leda::node u = VD.new_node();
                VD[u] = leda::rat_circle(isect);

                leda::edge r = VD.reversal(e);

                // NOTE: essentially sets target vertex,
                // does not change order of e in edge list of v
                VD.move_edge(e, v, u);

                VD.move_edge(r, u, v);
            }

            visited[e] = visited[VD.reversal(e)] = true;
        }

        e = VD.succ_edge(e);
        i++;
    }
    assert(de0);

    forall(v, outside) {
        if(!inBox[v]) VD.del_node(v);
    }

    // find start idx
    unsigned hand = 3;
    for(int i = 2; i >= 0; --i) {
        if(leda::left_turn(VD[VD.target(de0)].center(), boxVerts[hand], boxVerts[i]))
            hand = i;
    }
    std::cout << "starting hand = " << hand << std::endl;

    // connect

    leda::edge_map<int> mask(VD, 0);
    leda::edge it = de0;
    do {
        mask[it] = 1;
        it = VD.face_cycle_succ(it);
    } while(it != de0);
    mask[de0] = 2;

    leda::edge last_de = de0;
    it = NextDanglingEdge(VD, de0, NULL);
    while(de0 != it) {
        leda::edge next = NextDanglingEdge(VD, it, de0);

        leda::edge last_e = last_de;

        // connect box vertices
        while(leda::left_turn(VD[VD.target(last_de)].center(), VD[VD.target(it)].center(), boxVerts[hand]))
        {
            leda::node u = VD.new_node();
            VD[u] = circle_t(boxVerts[hand]);

            e = VD.new_edge(u, VD.target(last_e));
            leda::edge r = VD.new_edge(VD.reversal(last_e), u, leda::behind);
            VD.set_reversal(e, r);

            last_e = r;

            hand = (hand + 1) % 4;
        }

        e = VD.new_edge(VD.reversal(it), VD.target(last_e), leda::before);
        leda::edge r = VD.new_edge(VD.reversal(last_e), VD.target(it), leda::behind);
        VD.set_reversal(e, r);

        last_de = it;
        it = next;
    }

    leda::edge last_e = last_de;

    // connect box vertices
    while(leda::left_turn(VD[VD.target(last_de)].center(), VD[VD.target(it)].center(), boxVerts[hand]))
    {
        leda::node u = VD.new_node();
        VD[u] = circle_t(boxVerts[hand]);

        e = VD.new_edge(u, VD.target(last_e));
        leda::edge r = VD.new_edge(VD.reversal(last_e), u, leda::behind);
        VD.set_reversal(e, r);

        last_e = r;

        hand = (hand + 1) % 4;
    }

    e = VD.new_edge(VD.reversal(it), VD.target(last_e), leda::before);
    leda::edge r = VD.new_edge(VD.reversal(last_e), VD.target(it), leda::behind);
    VD.set_reversal(e, r);

    leda::edge_array<leda::edge> emap;
    out.clear();
    FromProjection(VD, out, emap);

    site.init(out, NULL);

    forall_edges(e, VD) {
        if(1 == mask[e]) out.set_color(emap[e], R::Color::Red);
        else if(2 == mask[e]) out.set_color(emap[e], R::Color::Blue);

        site[emap[e]] = FindVertexByPosition2(in, VD[e]);
    }

    return r;
}

void VDH_Operator::Register(Invoker& invoker) {
    AddMenuItem(AlgorithmMenu(), "VDH Demo", invoker);
}

static Point3 ProjectXY(const Point3& p) {
    return Point3(p.xcoord(), p.ycoord(), 0);
}

static leda::d3_rat_point ProjectOnParaboloid(const leda::d3_rat_point& p) {
    const leda::rational z = p.xcoord() * p.xcoord() + p.ycoord() * p.ycoord();
    return leda::d3_rat_point(p.xcoord(), p.ycoord(), z);
}

// assumes z(p0) = z(p1) = z(p2) = 0
static bool IsFrontFace(Graph& G, face f) {
    edge e = G.first_face_edge(f);
    Point3 p0 = G.position_of(source(e));
    Point3 p1 = G.position_of(target(e));

    edge it = G.face_cycle_succ(e);
    while(collinear(p0, p1, G.position_of(target(it))))
        it = G.face_cycle_succ(it);

    Point3 p2 = G.position_of(target(it));

    return 0 < orientation(p0, p1, p2, Point3(0, 0, 1));
}

void VDH_Operator::ApplyVoronoiColors() {
    NB::Graph& G = NB::GetGraph(_voronoiMesh);
    leda::edge e0, e;
    leda::face f;
    forall_faces(f, G) {
        if(G.is_visible(f)) {
            e0 = e = G.first_face_edge(f);
            while(!_site[e]) {
                e = G.face_cycle_succ(e);
                COM_assert(e0 != e);
            }
            G.set_color(f, _vertexColors[_site[e]]);
        }
    }
}

static void ConvexHull3D(const NB::Graph& verticesGraph, NB::Graph& hullGraph) {
    leda::list<leda::d3_rat_point> L;

    leda::node v;
    forall_nodes(v, verticesGraph) {
        L.push(ProjectOnParaboloid(verticesGraph.position_of(v)));
    }

    hullGraph.clear();
    leda::CONVEX_HULL(L, hullGraph);
    hullGraph.compute_faces();
}

void VDH_Operator::Update() {
    NB::Graph& verticesGraph = NB::GetGraph(_verticesMesh);

    Delaunay2D(verticesGraph, NB::GetGraph(_delaunayMesh));

    NB::Graph& voronoiGraph = NB::GetGraph(_voronoiMesh);

    // Voronoi2D(verticesGraph, NB::GetGraph(_voronoiMesh));
    leda::edge r = Voronoi2D_BruteForce(verticesGraph, NB::GetGraph(_voronoiMesh), _site);
    voronoiGraph.compute_faces();

    leda::face f;
    forall_faces(f, voronoiGraph) {
        if(!IsFrontFace(voronoiGraph, f)) voronoiGraph.set_visible(f, false);
    }
    /*
    voronoiGraph.set_visible(voronoiGraph.face_of(r), false);
    printf("IsFrontFace = %d\n", IsFrontFace(voronoiGraph, voronoiGraph.face_of(r)));
    */

    ApplyVoronoiColors();

    ConvexHull3D(verticesGraph, NB::GetGraph(_hullMesh));
}

static float RandFloat(float min, float max) {
    const float f = 1.0f / RAND_MAX;
    float t = (rand() % RAND_MAX) * f;
    return min + t * (max - min);
}

static void AssignVertexColors(NB::Graph& G, leda::node_array<R::Color>& colors) {
    const float f = 1.0f / 255.0f;
    const float r2 = f * 176;
    const float g2 = f * 196;
    const float b2 = f * 222;

    colors.init(G);

    leda::node v;
    forall_nodes(v, G) {
        float cr = (RandFloat(0.5f, 1.0f) + r2) * 0.5f;
        float cg = (RandFloat(0.5f, 1.0f) + g2) * 0.5f;
        float cb = (RandFloat(0.5f, 1.0f) + b2) * 0.5f;
        R::Color c;
        c.r = cr;
        c.g = cg;
        c.b = cb;
        c.a = 1.0f;
        colors[v] = c;
    }
}

void VDH_Operator::Event_CheckBoxToggled(const EV::Arg<bool>& event) {
    bool isChecked = event.value;

    switch(event.id1) {
    case ID_DELAUNAY:
        if(isChecked) NB::ShowMesh(_delaunayMesh);
        else NB::HideMesh(_delaunayMesh);
        break;
    case ID_VORONOI:
        if(isChecked) NB::ShowMesh(_voronoiMesh);
        else NB::HideMesh(_voronoiMesh);
        break;
    case ID_CHULL:
        if(isChecked) NB::ShowMesh(_hullMesh);
        else NB::HideMesh(_hullMesh);
        break;
    case ID_PARABOLOID:
        if(isChecked) NB::ShowMesh(_paraboloidMesh);
        else NB::HideMesh(_paraboloidMesh);
        break;
    };
}

VDH_Operator::VDH_Operator()
    : _verticesMesh(NULL)
    , _delaunayMesh(NULL)
    , _hullMesh(NULL)
    , _paraboloidMesh(NULL)
{
    AddEventHandler(ev_checkBoxToggled, this, &VDH_Operator::Event_CheckBoxToggled);
}

static leda::node MinXY(const NB::Graph& G, const leda::list<leda::node>& L) {
    leda::node min = L.front();
    for(unsigned i = 1; i < L.size(); ++i) {
        leda::node other = L[L.get_item(i)];
        if(G[other].xcoord() < G[min].xcoord() || (G[other].xcoord() == G[min].xcoord() && G[other].ycoord() < G[min].ycoord()))
            min = other;
    }
    return min;
}

static leda::edge FindOuterFace(NB::Graph& G) {
    leda::list<leda::node> L = G.all_nodes();
    leda::node n0 = MinXY(G, L);
    leda::edge e0 = NULL;
    leda::edge e;
    forall_out_edges(e, n0) {
        bool outerFace = true;
        leda::node n;
        forall_nodes(n, G) {
            if(0 < G.outdeg(n) && 0 < G.indeg(n) && 0 < leda::orientation_xy(G[leda::source(e)], G[leda::target(e)], G[n]))
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

bool VDH_Operator::Invoke() {
    node v, w;

    SetOperatorName("VDH Demo");

    NB::Mesh inputMesh = FirstSelectedMesh();
    if(!inputMesh) {
        LogPrintf("no input mesh selected.\n");
        return false;
    }

    NB::ClearSelection(); // HACKY HACK HACK

    NB::HideMesh(inputMesh);

    // create mesh for projected vertices
    _verticesMesh = NB::CreateMesh();
    NB::SetMeshName(_verticesMesh, "Projected Vertices");
    NB::SetMeshRenderMode(_verticesMesh, NB::RM_NODES);

    // copy input mesh, project all vertices onto XY plane
    const NB::Graph& inputGraph = NB::GetGraph(inputMesh);
    NB::Graph verticesGraph;
    forall_nodes(v, inputGraph) {
        w = verticesGraph.new_node();
        verticesGraph.set_position(w, ProjectXY(inputGraph.position_of(v)));
    }
    NB::SetGraph(_verticesMesh, verticesGraph);

    AssignVertexColors(NB::GetGraph(_verticesMesh), _vertexColors);

    // create mesh for projected delaunay triangulation
    _delaunayMesh = NB::CreateMesh();
    NB::SetMeshName(_delaunayMesh, "Delaunay Triangulation");
    NB::SetMeshRenderMode(_delaunayMesh, NB::RM_NODES | NB::RM_EDGES);
    NB::SetMeshEdgeTint(_delaunayMesh, R::Color::Blue);

    // create mesh for projected voronoi diagram
    _voronoiMesh = NB::CreateMesh();
    NB::SetMeshName(_voronoiMesh, "Voronoi Diagram");
    NB::SetMeshRenderMode(_voronoiMesh, NB::RM_ALL);

    // create mesh for convex hull
    _hullMesh = NB::CreateMesh();
    NB::SetMeshName(_hullMesh, "Convex Hull");
    NB::SetMeshRenderMode(_hullMesh, NB::RM_ALL);
    NB::SetMeshPosition(_hullMesh, M::Vector3(0.0f, 0.0f, 2.0f));

    // create mesh for paraboloid
    _paraboloidMesh = NB::CreateMesh();
    NB::SetMeshRenderMode(_paraboloidMesh, NB::RM_FACES);
    NB::SetMeshName(_paraboloidMesh, "Paraboloid");
    NB::SetMeshPosition(_paraboloidMesh, NB::GetMeshPosition(_paraboloidMesh) + M::Vector3(0.0f, 0.0f, 2.0f));
    leda::nb::RatPolyMesh& parabMesh = NB::GetGraph(_paraboloidMesh);
    const float maxSize = 10.0f;
    leda::nb::make_grid(parabMesh, 6, maxSize);
    leda::nb::set_color(parabMesh, R::Color::Green);

    _vertexEditor.SetAxisFlags(NB::AF_X | NB::AF_Y);
    _vertexEditor.Open(_verticesMesh);

    Update();

    return true;
}

void VDH_Operator::Finish() {
    _vertexEditor.Close();
}

void VDH_Operator::OnMouse(const EV::MouseEvent& event) {
    if(_vertexEditor.HandleMouseEvent(event)) {
        Update();
        event.Accept();
    }
}

} // namespace OP