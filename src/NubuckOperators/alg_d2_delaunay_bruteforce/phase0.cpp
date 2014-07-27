#include <Nubuck\polymesh.h>
#include "phase_flip.h"
#include "phase0.h"

inline bool equal_xy(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
    return 0 == leda::d3_rat_point::cmp_x(lhp, rhp) && 0 == leda::d3_rat_point::cmp_y(lhp, rhp);
}

/*
====================
TriangulateXY
    precond: mesh does not contain any edges, L is the sorted node list of mesh.
    triangulates the mesh according to xy-order of vertices, returns edge of hull
====================
*/
// cnf. d3hf13.cpp (LEDA)
leda::edge TriangulateXY(leda::nb::RatPolyMesh& mesh, leda::list<leda::node>& L, int orient) {
    typedef leda::d3_rat_point point_t;

    // assert(0 == mesh.number_of_edges());
    // assert(IsSorted(mesh, L));

    if(L.empty()) return NULL;

    leda::node  last_v  = L.pop_front();
    point_t     last_p  = mesh.position_of(last_v);

    while(!L.empty() && equal_xy(last_p, mesh[L.front()])) {
        mesh.del_node(L.pop_front());
    }

    if(!L.empty()) {
        leda::node v = L.pop_front();

        leda::edge e0 = mesh.new_edge(last_v, v, 0);
        leda::edge e1 = mesh.new_edge(v, last_v, 0);
        mesh.set_reversal(e0, e1);
        SetColorU(mesh, e0, Color::BLACK);

        last_v = v;
        last_p = mesh.position_of(v);
    }

    // scan remaining points

    leda::node v;
    forall(v, L) {
        point_t p = mesh.position_of(v);

        if(equal_xy(p, last_p)) {
            mesh.del_node(v);
            continue;
        }

        // walk up to upper tangent
        leda::edge e = mesh.last_edge();
        int orientXY;
        do {
            e = mesh.face_cycle_pred(e);
            orientXY = leda::orientation_xy(
                p,
                mesh.position_of(leda::source(e)),
                mesh.position_of(leda::target(e)));
        } while(orient == orientXY);

        // walk down to lower tangent and triangulate
        do {
            leda::edge succ = mesh.face_cycle_succ(e);
            leda::edge x = mesh.new_edge(succ, v, 0, leda::after);
            leda::edge y = mesh.new_edge(v, leda::source(succ), 0);
            mesh.set_reversal(x, y);
            SetColorU(mesh, x, Color::BLACK);
            e = succ;

            orientXY = leda::orientation_xy(
                p,
                mesh.position_of(leda::source(e)),
                mesh.position_of(leda::target(e)));
        } while(orient == orientXY);

        last_p = p;
    } // forall nodes in L

    leda::edge hull = mesh.last_edge();
    leda::edge e = hull;
    do {
        SetColorU(mesh, e, Color::BLUE);
        e = mesh.face_cycle_succ(e);
    } while(hull != e);

    return mesh.last_edge();
}

struct CompareVertexPositionsDescending : leda::leda_cmp_base<leda::node> {
    const leda::nb::RatPolyMesh& mesh;

    CompareVertexPositionsDescending(const leda::nb::RatPolyMesh& mesh) : mesh(mesh) { }

    int operator()(const leda::node& lhp, const leda::node& rhp) const override {
        return -leda::compare(mesh.position_of(lhp), mesh.position_of(rhp));
    }
};

void Copy_MapAB(
    const leda::nb::RatPolyMesh& meshA,
    leda::nb::RatPolyMesh& meshB,
    leda::node_map<leda::node>& vmap,
    leda::edge_map<leda::edge>& emap)
{
    using namespace leda;

    meshB.clear();
    meshB.set_node_bound(meshA.number_of_nodes());
    meshB.set_edge_bound(meshA.number_of_edges());

    vmap.init(meshA, 0);
    emap.init(meshA, 0);

    node va, vb, wa;
    edge ea, eb, ra;

    forall_nodes(va, meshA) {
        vb = meshB.new_node();
        meshB.set_position(vb, meshA.position_of(va));
        vmap[va] = vb;
    }

    forall_nodes(va, meshA) {
        forall_adj_edges(ea, va) {
            wa = target(ea);
            eb = meshB.new_edge(vmap[va], vmap[wa]);
            emap[ea] = eb;
        }
    }

    edge_array<bool> visited(meshA, false);
    forall_edges(ea, meshA) {
        if(!visited[ea]) {
            ra = meshA.reversal(ea);
            meshB.set_reversal(emap[ea], emap[ra]);
            visited[ea] = visited[ra] = true;
        }
    }

    meshB.compute_faces();
}

leda::d3_rat_point ProjectOnParaboloid(const leda::d3_rat_point& p) {
    const leda::rational z = p.xcoord() * p.xcoord() + p.ycoord() * p.ycoord();
    return leda::d3_rat_point(p.xcoord(), p.ycoord(), z);
}

M::Vector3 ProjectOnParaboloid_F(const M::Vector3& v) {
    return M::Vector3(v.x, v.y, v.x * v.x + v.y * v.y);
}

leda::d3_rat_point ToRatPoint(const M::Vector3& v) {
    return leda::d3_rat_point(leda::d3_point(v.x, v.y, v.z));
}

// somehow this doesn't work when using exact arithmetic. oh well...
void CreateGrid(
    leda::nb::RatPolyMesh& mesh,
    const int subdiv,
    const float size)
{
    const leda::rational zero(0, 1);

    mesh.clear();

    const int N = (1 << subdiv) + 1;

    leda::node* nodes = new leda::node[N * N];

    const float         hsize = 0.5f * size;
    const M::Vector3    off = M::Vector3(hsize, hsize, 0.0f);
    const float         ds = size / (N - 1);

    float x = 0.0f, y = 0.0f;

    for(unsigned i = 0; i < N; ++i) {
        y = 0.0f;
        for(unsigned j = 0; j < N; ++j) {
            const leda::node v = mesh.new_node();
            M::Vector3 pos = M::Vector3(x, y, 0.0f) - off;
            mesh.set_position(v, ToRatPoint(ProjectOnParaboloid_F(pos)));
            nodes[N * i + j] = v;

            y += ds;
        }

        x += ds;
    }

    for(unsigned i = 0; i < N; ++i) {
        for(unsigned j = 0; j < N; ++j) {
            if(i + 1 < N)   mesh.new_edge(nodes[N * i + j], nodes[N * (i + 1) + j]);
            if(j + 1 < N)   mesh.new_edge(nodes[N * i + j], nodes[N * i + (j + 1)]);
            if(i > 0)       mesh.new_edge(nodes[N * i + j], nodes[N * (i - 1) + j]);
            if(j > 0)       mesh.new_edge(nodes[N * i + j], nodes[N * i + (j - 1)]);
        }
    }

    delete[] nodes;

    bool isBidirected = mesh.make_map();
    assert(isBidirected);
    mesh.compute_faces();

    mesh.set_visible(mesh.face_of(mesh.reversal(mesh.first_edge())), false);
}

void Phase0::Enter() {
    _g.nb.log->printf("entering phase 'init'\n");

    const int renderMode =
        IGeometry::RenderMode::NODES |
        IGeometry::RenderMode::EDGES;
    _g.delaunay->SetRenderMode(renderMode);

    leda::nb::RatPolyMesh& mesh = _g.delaunay->GetRatPolyMesh();

    leda::list<leda::node> L = mesh.all_nodes();

    L.sort(CompareVertexPositionsDescending(mesh));

    _g.hullEdge = TriangulateXY(mesh, L, -1);

    mesh.compute_faces();
    mesh.set_visible(mesh.face_of(_g.hullEdge), false);

    ApplyEdgeColors(mesh);

    // compute size for paraboloid
    float maxSize = 0.0f;

    leda::node v;
    forall_nodes(v, mesh) {
        maxSize = M::Max(maxSize, (float)mesh.position_of(v).xcoord().to_float());
        maxSize = M::Max(maxSize, (float)mesh.position_of(v).ycoord().to_float());
    }
    maxSize += 1.0f;
    maxSize *= 2.0f;

    // copy mesh
    const int renderAll =
        IGeometry::RenderMode::NODES |
        IGeometry::RenderMode::EDGES |
        IGeometry::RenderMode::FACES;
    _g.chull = _g.nb.world->CreateGeometry();
    _g.chull->SetName("Convex Hull");
    _g.chull->SetRenderMode(renderAll);
    _g.chull->SetPosition(_g.chull->GetPosition() + M::Vector3(0.0f, 0.0f, 2.0f));
    _g.chull->Hide();

    leda::nb::RatPolyMesh& chullMesh = _g.chull->GetRatPolyMesh();

    Copy_MapAB(mesh, chullMesh, _g.vmap, _g.emap);

    chullMesh.set_visible(chullMesh.face_of(_g.emap[_g.hullEdge]), false);

    forall_nodes(v, chullMesh) {
        chullMesh.set_position(v, ProjectOnParaboloid(chullMesh.position_of(v)));
    }

    // paraboloid mesh
    _g.paraboloid = _g.nb.world->CreateGeometry();
    _g.paraboloid->SetRenderMode(IGeometry::RenderMode::FACES);
    _g.paraboloid->SetName("Paraboloid");
    _g.paraboloid->SetPosition(_g.paraboloid->GetPosition() + M::Vector3(0.0f, 0.0f, 2.0f));
    _g.paraboloid->Hide();

    leda::nb::RatPolyMesh& parabMesh = _g.paraboloid->GetRatPolyMesh();

    CreateGrid(parabMesh, 6, maxSize);

    leda::nb::set_color(parabMesh, R::Color::Green);
}

Phase0::StepRet::Enum Phase0::Step() {
    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase0::NextPhase() {
    return GEN::MakePtr(new Phase_Flip(_g));
}