#include <Nubuck\polymesh.h>
#include "phase_init.h"
#include "phase_flip.h"
#include "phase_strip.h"
#include "phase_clip.h"

namespace {

void SimplifyFace(leda::nb::RatPolyMesh& mesh, leda::node v) {
    if(mesh.outdeg(v) <= 3) return; // nothing to do

    leda::edge e1 = mesh.first_adj_edge(v);
    leda::edge e2 = mesh.cyclic_adj_succ(e1);
    leda::edge e3 = mesh.cyclic_adj_succ(e2);

    // count: number of traversed edges since last flip
    int count = 0;
    while(count++ < mesh.outdeg(v) && mesh.outdeg(v) > 3) {
        leda::d3_rat_point p0 = mesh.position_of(v);
        leda::d3_rat_point p1 = mesh.position_of(leda::target(e1));
        leda::d3_rat_point p2 = mesh.position_of(leda::target(e2));
        leda::d3_rat_point p3 = mesh.position_of(leda::target(e3));

        int orient_130 = leda::orientation_xy(p1, p3, p0);
        int orient_132 = leda::orientation_xy(p1, p3, p2);

        if(orient_130 != orient_132 && orient_132 != 0) {
            leda::edge r2 = mesh.reversal(e2);
            mesh.move_edge(e2, mesh.reversal(e1), leda::target(e3), leda::before);
            mesh.move_edge(r2, mesh.reversal(e3), leda::target(e1));
            mesh[e2] = mesh[e2] = Color::RED;
            count = 0;
        } else {
            e1 = e2;
        }

        e2 = e3;
        e3 = mesh.cyclic_adj_succ(e2);
    }

    assert(mesh.outdeg(v) <= 3);
}

} // unnamed namespace

Phase_Clip::Phase_Clip(Globals& g, bool forceFlips)
    : _g(g)
    , _forceFlips(forceFlips)
{ }

void Phase_Clip::Enter() {
    _g.nb.log->printf("entering phase 'clip'\n");

    leda::nb::RatPolyMesh& mesh = _g.geom[_g.side]->GetRatPolyMesh();

    _L.clear();
    _rdeg.init(mesh, 0);
    _cdeg.init(mesh, 0);
    leda::edge e;
    forall_edges(e, mesh) {
        leda::node v = leda::source(e);
        if(Color::RED == mesh[e]) {
            if(3 == ++_rdeg[v]) _L.push(v);
        } else {
            _cdeg[v]++;
        }
    }

    _numClips = 0;
    _stepMode = StepMode::SEARCH;
}

Phase_Clip::StepRet::Enum Phase_Clip::StepSearch() {
    leda::nb::RatPolyMesh& mesh = _g.geom[_g.side]->GetRatPolyMesh();

    while(!_L.empty()) {
        _clipV = _L.pop();

        if(_cdeg[_clipV]) continue;

        if(RunMode::STEP == GetRunConf().mode) {
            // highlight neighbourhood of vertex v
            leda::nb::set_color(mesh, R::Color(1.0f, 1.0f, 1.0f, 0.2f));
            leda::edge e;
            forall_out_edges(e, _clipV) {
                mesh.set_color(mesh.face_of(e), R::Color::Red);
            }
            mesh.set_color(_clipV, R::Color::Yellow);
        }

        _stepMode = StepMode::PERFORM_CLIP;
        return StepRet::CONTINUE;
    }

    if(RunMode::NEXT == GetRunConf().mode) {
        mesh.compute_faces();
        if(_g.hullEdges[Side::FRONT]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::BACK]), false);
        ApplyEdgeColors(mesh);
    }

    _g.nb.log->printf("number of clips: %d\n", _numClips);
    return StepRet::DONE;
}

Phase_Clip::StepRet::Enum Phase_Clip::StepPerformClip() {
    leda::nb::RatPolyMesh& mesh = _g.geom[_g.side]->GetRatPolyMesh();

    leda::edge e;
    forall_out_edges(e, _clipV) {
        leda::edge b = mesh.face_cycle_succ(e); // boundary edge
        if(Color::BLACK == mesh[b]) {
            mesh[b] = mesh[mesh.reversal(b)] = Color::RED;
        }
    }

    if(3 < mesh.outdeg(_clipV)) SimplifyFace(mesh, _clipV);

    mesh.del_node(_clipV);
    _numClips++;

    if(RunMode::STEP == GetRunConf().mode) {
        mesh.compute_faces();
        if(_g.hullEdges[Side::FRONT]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::BACK]), false);
        ApplyEdgeColors(mesh);
    }

    _stepMode = StepMode::SEARCH;
    return StepRet::CONTINUE;
}

Phase_Clip::StepRet::Enum Phase_Clip::Step() {
    if(StepMode::SEARCH == _stepMode) {
        return StepSearch();
    } else {
        return StepPerformClip();
    }
}

GEN::Pointer<OP::ALG::Phase> Phase_Clip::NextPhase() {
    if(_forceFlips || _numClips) {
        return GEN::MakePtr(new Phase_Flip(_g));
    } else {
        return GEN::MakePtr(new Phase_Strip(_g));
    }
}