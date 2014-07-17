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

void Phase0::Enter() {
    _g.nb.log->printf("entering phase 'init'\n");

    const int renderMode =
        IGeometry::RenderMode::NODES |
        IGeometry::RenderMode::EDGES;
    _g.inputGeom->SetRenderMode(renderMode);

    leda::nb::RatPolyMesh& mesh = _g.inputGeom->GetRatPolyMesh();

    leda::list<leda::node> L = mesh.all_nodes();

    L.sort(CompareVertexPositionsDescending(mesh));

    leda::edge hullEdge = TriangulateXY(mesh, L, -1);

    mesh.compute_faces();
    mesh.set_visible(mesh.face_of(hullEdge), false);

    ApplyEdgeColors(mesh);
}

Phase0::StepRet::Enum Phase0::Step() {
    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase0::NextPhase() {
    return GEN::MakePtr(new Phase_Flip(_g));
}