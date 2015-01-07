#include <Nubuck\polymesh.h>
#include "phase_simplify.h"
#include "phase_stitch.h"

bool IsCollinear(leda::nb::RatPolyMesh& graph, leda::edge e) {
    leda::edge r = graph.reversal(e);

    leda::edge e1 = graph.face_cycle_succ(r);
    leda::edge e3 = graph.face_cycle_succ(e);

    const leda::node v0 = leda::source(e1);
    const leda::node v1 = leda::target(e1);
    const leda::node v2 = leda::source(e3);
    const leda::node v3 = leda::target(e3);

    const leda::d3_rat_point p0 = graph.position_of(v0);
    const leda::d3_rat_point p1 = graph.position_of(v1);
    const leda::d3_rat_point p2 = graph.position_of(v2);
    const leda::d3_rat_point p3 = graph.position_of(v3);

    return 0 == leda::orientation(p0, p1, p2, p3);
}

inline bool equal_xy(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
    return 0 == leda::d3_rat_point::cmp_x(lhp, rhp) && 0 == leda::d3_rat_point::cmp_y(lhp, rhp);
}

Phase_Stitch::Phase_Stitch(Globals& g) : _g(g) { }

void Phase_Stitch::Enter() {
    NB::LogPrintf("entering phase 'stitch'\n");

    if(RunMode::RUN == GetRunConf().mode && _g.haltBeforeStitching) {
        // previous phase might have not computed faces in run mode
        for(int side = 0; side < 2; ++side) {
            leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[side]);
            graph.compute_faces();
            ApplyEdgeColors(graph);
            if(_g.hullEdges[side]) graph.set_visible(graph.face_of(_g.hullEdges[side]), false);
        }
    }
}

Phase_Stitch::StepRet::Enum Phase_Stitch::Step() {
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[Side::FRONT]);

    graph.join(NB::GetGraph(_g.meshes[Side::BACK]));

    NB::DestroyMesh(_g.meshes[Side::BACK]);
    NB::SetMeshName(_g.meshes[Side::FRONT], "Stitched Hull");

    int numHullVerts = 0;
    leda::edge e = _g.hullEdges[Side::FRONT];
    do {
        numHullVerts++;
        e = graph.face_cycle_succ(e);
    } while(_g.hullEdges[Side::FRONT] != e);

    leda::edge adv0 = _g.hullEdges[Side::FRONT];
    while(_g.stitchVerts[Side::FRONT] != leda::target(adv0))
        adv0 = graph.face_cycle_pred(adv0); // here dir is arbitrary


    leda::edge adv1 = _g.hullEdges[Side::BACK];
    while(_g.stitchVerts[Side::BACK] != leda::source(adv1))
        adv1 = graph.face_cycle_succ(adv1); // here dir is arbitrary

    leda::node v0, v1;
    do {
        v0 = leda::target(adv0);
        v1 = leda::source(adv1);

        assert(equal_xy(graph.position_of(v0), graph.position_of(v1)));

        if(0 != leda::d3_rat_point::cmp_z(graph.position_of(v0), graph.position_of(v1))) {
            // stitch

            int dir = leda::before;
            leda::edge e = graph.new_edge(graph.reversal(adv0), v1, dir, dir);
            leda::edge r = graph.new_edge(graph.cyclic_adj_succ(adv1), v0, dir, dir);
            graph.set_reversal(e, r);

            if(IsCollinear(graph, e)) {
                MarkPlanar(graph, e);
            }

            adv0 = graph.face_cycle_pred(adv0);
            adv1 = graph.face_cycle_succ(adv1);
        } else {
            // collapse

            leda::edge succ0    = graph.face_cycle_succ(adv0);
            leda::edge term1    = graph.cyclic_adj_succ(adv1);

            leda::edge r, next, it1;

            it1  = adv1;
            next = graph.cyclic_adj_pred(it1);

            if(leda::source(adv0) != leda::target(it1)) {
                r = graph.reversal(it1);
                graph.move_edge(it1, succ0, leda::target(it1), leda::behind);
                graph.move_edge(r, leda::source(r), v0);
            }
            it1 = next;

            while(it1 != term1) {
                next = graph.cyclic_adj_pred(it1);
                r = graph.reversal(it1);
                graph.move_edge(it1, succ0, leda::target(it1), leda::behind);
                graph.move_edge(r, leda::source(r), v0);
                it1 = next;
            }

            if(leda::target(succ0) != leda::target(it1)) {
                r = graph.reversal(it1);
                graph.move_edge(it1, succ0, leda::target(it1), leda::behind);
                graph.move_edge(r, leda::source(r), v0);
            }

            adv0 = graph.face_cycle_pred(adv0);
            adv1 = graph.face_cycle_succ(adv1);

            graph.del_node(v1);
        } // collapse

    } while(_g.stitchVerts[Side::FRONT] != leda::target(adv0));

    graph.compute_faces();

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase_Stitch::NextPhase() {
    return GEN::MakePtr(new Phase_Simplify(_g));
}

bool Phase_Stitch::IsWall() const {
    return _g.haltBeforeStitching;
}