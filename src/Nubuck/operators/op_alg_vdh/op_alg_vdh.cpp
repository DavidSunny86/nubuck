#include <maxint.h>

#include <LEDA\geo\geo_alg.h>
#include <LEDA\geo\d3_hull.h>
#include <Nubuck\animation\move_vertex_anim.h>
#include "op_alg_vdh.h"

using namespace leda;
using namespace NB;

typedef leda::rat_point             point2_t;
typedef leda::GRAPH<point2_t, int>  graph2_t;

namespace OP {

static leda::list<point2_t> ToPointList2(const NB::Graph& G) {
    leda::list<point2_t> L;
    leda::node n;
    forall_nodes(n, G) L.push_back(point2_t(G[n].xcoord(), G[n].ycoord()));
    return L;
}

template<typename IN_ETYPE, typename OUT_ETYPE>
static void FromProjection(const leda::GRAPH<leda::rat_point, IN_ETYPE>& in, leda::GRAPH<leda::d3_rat_point, OUT_ETYPE>& out, leda::edge_array<leda::edge>& emap) {
    leda::node_array<leda::node>  nmap(in, NULL);
    emap.init(in, NULL);

    leda::node n;
    forall_nodes(n, in) {
        nmap[n] = out.new_node();
        out[nmap[n]] = leda::d3_rat_point(in[n].xcoord(), in[n].ycoord(), 0);
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

static void CreateBoundedSegmentation(
    const leda::GRAPH<leda::rat_circle, point2_t>& VD, 
    leda::GRAPH<leda::rat_point, leda::rat_segment>& G2)
{
    leda::list<leda::rat_segment> S0;
    leda::list<leda::rat_segment> S1;

    leda::rational inf = 500;
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

static void Voronoi2D(const NB::Graph& in, leda::GRAPH<leda::rat_point, int>& TRI, NB::Graph& out, leda::edge_map<leda::edge>& map) {
    leda::list<point2_t> L(ToPointList2(in));
    L.unique();

    leda::GRAPH<leda::rat_circle, point2_t> VD;
    leda::VORONOI(L, VD);

    leda::GRAPH<leda::rat_point, leda::rat_segment> G2;
    CreateBoundedSegmentation(VD, G2);

    TRI.clear();
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
    Point3 p2 = G.position_of(target(G.face_cycle_succ(e)));
    return 0 < orientation(p0, p1, p2, Point3(0, 0, 1));
}

void VDH_Operator::ApplyVoronoiColors() {
    const NB::Graph& verticesGraph = NB::GetGraph(_verticesMesh);
    NB::Graph& voronoiGraph = NB::GetGraph(_voronoiMesh);

    leda::node v;
    forall_nodes(v, verticesGraph) {
        point2_t p(verticesGraph.position_of(v).xcoord(), verticesGraph.position_of(v).ycoord());
        leda::edge ft = leda::LOCATE_IN_TRIANGULATION(_voronoiTriang, p);
        leda::face f = voronoiGraph.face_of(_voronoiMap[ft]);
        R::Color c = _vertexColors[v];
        voronoiGraph.set_color(f, c);
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

    Voronoi2D(verticesGraph, _voronoiTriang, NB::GetGraph(_voronoiMesh), _voronoiMap);
    voronoiGraph.compute_faces();

    leda::face f;
    forall_faces(f, voronoiGraph) {
        if(!IsFrontFace(voronoiGraph, f)) voronoiGraph.set_visible(f, false);
    }

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

VDH_Operator::VDH_Operator()
    : _verticesMesh(NULL)
    , _delaunayMesh(NULL)
{ }

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

    // create mesh for projected voronoi diagram
    _voronoiMesh = NB::CreateMesh();
    NB::SetMeshName(_voronoiMesh, "Voronoi Diagram");
    NB::SetMeshRenderMode(_voronoiMesh, NB::RM_ALL);

    // create mesh for convex hull
    _hullMesh = NB::CreateMesh();
    NB::SetMeshName(_hullMesh, "Convex Hull");
    NB::SetMeshRenderMode(_hullMesh, NB::RM_ALL);
    NB::SetMeshPosition(_hullMesh, M::Vector3(0.0f, 0.0f, 2.0f));

    _vertexEditor.SetAxisFlags(NB::AF_X | NB::AF_Y);
    _vertexEditor.Open(_verticesMesh);

    Update();

    return true;
}

void VDH_Operator::Finish() {
}

void VDH_Operator::OnMouse(const EV::MouseEvent& event) {
    if(_vertexEditor.HandleMouseEvent(event)) {
        Update();
        event.Accept();
    }
}

} // namespace OP