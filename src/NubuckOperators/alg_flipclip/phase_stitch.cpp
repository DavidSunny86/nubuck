#include <Nubuck\polymesh.h>
#include "phase_simplify.h"
#include "phase_stitch.h"

static bool EdgeEx(leda::nb::RatPolyMesh& mesh, leda::node v0, leda::node v1) {
    leda::edge e, e0 = nil;
    forall_out_edges(e, v0) {
        if(mesh.target(e) == v1) return true;
    }
    return false;
}

// source(fhe) == source(bhe)
static void StitchVertices(leda::nb::RatPolyMesh& mesh, leda::edge fhe, leda::edge bhe) {
    leda::node fv = leda::source(fhe);
    leda::node bv = leda::source(bhe);

    int x = leda::before;
    leda::edge e = mesh.new_edge(mesh.cyclic_adj_succ(bhe), fv, x, x);
    leda::edge r = mesh.new_edge(mesh.cyclic_adj_succ(fhe), bv, x, x);
    mesh.set_reversal(e, r);
}

// TODO: optimize me!
static leda::node merge_vertices(leda::nb::RatPolyMesh& mesh, leda::node v0, leda::node v1) {
    leda::edge e;

    leda::edge e01 = v0->first_out_edge();
    while(e01 && e01->terminal(1) != v1)
        e01 = e01->next_out_edge();
    assert(e01 && e01->terminal(1) == v1);

    leda::edge e10 = mesh.reversal(e01);

    leda::edge i = mesh.cyclic_adj_pred(e01);
    leda::edge j = mesh.cyclic_adj_pred(e10);

    leda::list<leda::edge> del;
    while(j != e10) {
        leda::node w = leda::target(j);
        leda::edge r = mesh.reversal(j);
        if(!EdgeEx(mesh, v0, w)) {
            leda::edge e = mesh.new_edge(i, w);
            r->set_term(1, v0);
            mesh.set_reversal(e, r);
        } else {
            del.push(r);
        }
        j = mesh.cyclic_adj_pred(j);
    }

    forall(e, del) mesh.del_edge(e);
    mesh.del_edge(e01);

    v0->append_adj_list(1, v1, 1);
    mesh.del_node(v1);

    assert(mesh.is_bidirected());
    assert(mesh.is_map());

    return v0;
}

inline bool equal_xy(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
    return 0 == leda::d3_rat_point::cmp_x(lhp, rhp) && 0 == leda::d3_rat_point::cmp_y(lhp, rhp);
}

Phase_Stitch::Phase_Stitch(Globals& g) : _g(g) { }

void Phase_Stitch::Enter() {
    _g.nb.log->printf("entering phase 'stitch'\n");

    if(RunMode::RUN == GetRunConf().mode && _g.haltBeforeStitching) {
        // previous phase might have not computed faces in run mode
        for(int side = 0; side < 2; ++side) {
            leda::nb::RatPolyMesh& mesh = _g.geom[side]->GetRatPolyMesh();
            mesh.compute_faces();
            ApplyEdgeColors(mesh);
            if(_g.hullEdges[side]) mesh.set_visible(mesh.face_of(_g.hullEdges[side]), false);
        }
    }
}

Phase_Stitch::StepRet::Enum Phase_Stitch::Step() {
    leda::nb::RatPolyMesh& mesh = _g.geom[Side::FRONT]->GetRatPolyMesh();

    mesh.join(_g.geom[Side::BACK]->GetRatPolyMesh());

    _g.geom[Side::BACK]->Destroy();
    _g.geom[Side::FRONT]->SetName("Stitched Hull");

    int numHullVerts = 0;
    leda::edge e = _g.hullEdges[Side::FRONT];
    do {
        numHullVerts++;
        e = mesh.face_cycle_succ(e);
    } while(_g.hullEdges[Side::FRONT] != e);

    leda::edge fhe = _g.hullEdges[Side::FRONT];
    while(_g.stitchVerts[Side::FRONT] != leda::source(fhe))
        fhe = mesh.face_cycle_succ(fhe);


    leda::edge bhe = _g.hullEdges[Side::BACK];
    while(_g.stitchVerts[Side::BACK] != leda::source(bhe))
        bhe = mesh.face_cycle_succ(bhe);

    for(int i = 0; i < numHullVerts; ++i) {
        leda::node fv = leda::source(fhe);
        leda::node bv = leda::source(bhe);

        assert(equal_xy(mesh.position_of(fv), mesh.position_of(bv)));

        leda::edge bn = mesh.face_cycle_pred(bhe);
        leda::edge fn = mesh.face_cycle_succ(fhe);

        StitchVertices(mesh, fhe, bhe);

        leda::d3_rat_point fp = mesh.position_of(leda::source(fhe));
        leda::d3_rat_point bp = mesh.position_of(leda::source(bhe));

        if(0 == leda::d3_rat_point::cmp_z(fp, bp)) {
            merge_vertices(mesh, fv, bv);

            bhe = mesh.face_cycle_pred(fhe);
        } else bhe = bn;

        fhe = fn;
    }

    mesh.compute_faces();

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase_Stitch::NextPhase() {
    return GEN::MakePtr(new Phase_Simplify(_g));
}

bool Phase_Stitch::IsWall() const {
    return _g.haltBeforeStitching;
}