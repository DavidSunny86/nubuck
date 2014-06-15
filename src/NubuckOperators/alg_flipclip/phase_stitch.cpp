#include <Nubuck\polymesh.h>
#include "phase_simplify.h"
#include "phase_stitch.h"

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

    leda::edge adv0 = _g.hullEdges[Side::FRONT];
    while(_g.stitchVerts[Side::FRONT] != leda::target(adv0))
        adv0 = mesh.face_cycle_pred(adv0); // here dir is arbitrary


    leda::edge adv1 = _g.hullEdges[Side::BACK];
    while(_g.stitchVerts[Side::BACK] != leda::source(adv1))
        adv1 = mesh.face_cycle_succ(adv1); // here dir is arbitrary

    leda::node v0, v1;
    do {
        v0 = leda::target(adv0);
        v1 = leda::source(adv1);

        assert(equal_xy(mesh.position_of(v0), mesh.position_of(v1)));

        if(0 != leda::d3_rat_point::cmp_z(mesh.position_of(v0), mesh.position_of(v1))) {
            // stitch

            int dir = leda::before;
            leda::edge e = mesh.new_edge(mesh.reversal(adv0), v1, dir, dir);
            leda::edge r = mesh.new_edge(mesh.cyclic_adj_succ(adv1), v0, dir, dir);
            mesh.set_reversal(e, r);

            adv0 = mesh.face_cycle_pred(adv0);
            adv1 = mesh.face_cycle_succ(adv1);
        } else {
            // collapse

            leda::edge succ0    = mesh.face_cycle_succ(adv0);
            leda::edge term1    = mesh.cyclic_adj_succ(adv1);

            leda::edge r, next, it1;

            it1  = adv1;
            next = mesh.cyclic_adj_pred(it1);

            if(leda::source(adv0) != leda::target(it1)) {
                r = mesh.reversal(it1);
                mesh.move_edge(it1, succ0, leda::target(it1), leda::behind);
                mesh.move_edge(r, leda::source(r), v0);
            }
            it1 = next;

            while(it1 != term1) {
                next = mesh.cyclic_adj_pred(it1);
                r = mesh.reversal(it1);
                mesh.move_edge(it1, succ0, leda::target(it1), leda::behind);
                mesh.move_edge(r, leda::source(r), v0);
                it1 = next;
            }

            if(leda::target(succ0) != leda::target(it1)) {
                r = mesh.reversal(it1);
                mesh.move_edge(it1, succ0, leda::target(it1), leda::behind);
                mesh.move_edge(r, leda::source(r), v0);
            }

            adv0 = mesh.face_cycle_pred(adv0);
            adv1 = mesh.face_cycle_succ(adv1);

            mesh.del_node(v1);
        } // collapse

    } while(_g.stitchVerts[Side::FRONT] != leda::target(adv0));

    mesh.compute_faces();

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase_Stitch::NextPhase() {
    return GEN::MakePtr(new Phase_Simplify(_g));
}

bool Phase_Stitch::IsWall() const {
    return _g.haltBeforeStitching;
}