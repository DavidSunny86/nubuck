#include <Nubuck\polymesh.h>
#include "phase_init.h"
#include "phase_flip.h"
#include "phase_strip.h"
#include "phase_clip.h"

namespace {

void SimplifyFace(leda::nb::RatPolyMesh& graph, leda::node v) {
    if(graph.outdeg(v) <= 3) return; // nothing to do

    leda::edge e1 = graph.first_adj_edge(v);
    leda::edge e2 = graph.cyclic_adj_succ(e1);
    leda::edge e3 = graph.cyclic_adj_succ(e2);

    // count: number of traversed edges since last flip
    int count = 0;
    while(count++ < graph.outdeg(v) && graph.outdeg(v) > 3) {
        leda::d3_rat_point p0 = graph.position_of(v);
        leda::d3_rat_point p1 = graph.position_of(leda::target(e1));
        leda::d3_rat_point p2 = graph.position_of(leda::target(e2));
        leda::d3_rat_point p3 = graph.position_of(leda::target(e3));

        int orient_130 = leda::orientation_xy(p1, p3, p0);
        int orient_132 = leda::orientation_xy(p1, p3, p2);

        if(orient_130 != orient_132 && orient_132 != 0) {
            leda::edge r2 = graph.reversal(e2);
            graph.move_edge(e2, graph.reversal(e1), leda::target(e3), leda::before);
            graph.move_edge(r2, graph.reversal(e3), leda::target(e1));
            InvalidateU(graph, e2);
            count = 0;
        } else {
            e1 = e2;
        }

        e2 = e3;
        e3 = graph.cyclic_adj_succ(e2);
    }

    assert(graph.outdeg(v) <= 3);
}

} // unnamed namespace

Phase_Clip::Phase_Clip(Globals& g, bool forceFlips)
    : _g(g)
    , _forceFlips(forceFlips)
{ }

void Phase_Clip::Enter() {
    NB::LogPrintf("entering phase 'clip'\n");

    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    _L.clear();
    _rdeg.init(graph, 0);
    _cdeg.init(graph, 0);
    leda::edge e;
    forall_edges(e, graph) {
        leda::node v = leda::source(e);
        if(Color::RED == GetColor(graph, e)) {
            if(3 == ++_rdeg[v]) _L.push(v);
        } else {
            _cdeg[v]++;
        }
    }

    _numClips = 0;
    _stepMode = StepMode::SEARCH;
}

Phase_Clip::StepRet::Enum Phase_Clip::StepSearch() {
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    while(!_L.empty()) {
        _clipV = _L.pop();

        if(_cdeg[_clipV]) continue;

        if(RunMode::STEP == GetRunConf().mode) {
            // highlight neighbourhood of vertex v
            leda::nb::set_color(graph, R::Color(1.0f, 1.0f, 1.0f, 0.2f));
            leda::edge e;
            forall_out_edges(e, _clipV) {
                graph.set_color(graph.face_of(e), R::Color::Red);
            }
            graph.set_color(_clipV, R::Color::Yellow);
        }

        _stepMode = StepMode::PERFORM_CLIP;
        return StepRet::CONTINUE;
    }

    if(RunMode::NEXT == GetRunConf().mode) {
        graph.compute_faces();
        if(_g.hullEdges[Side::FRONT]) graph.set_visible(graph.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) graph.set_visible(graph.face_of(_g.hullEdges[Side::BACK]), false);
        ApplyEdgeColors(graph);
    }

    NB::LogPrintf("number of clips: %d\n", _numClips);
    return StepRet::DONE;
}

Phase_Clip::StepRet::Enum Phase_Clip::StepPerformClip() {
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    leda::edge e;
    forall_out_edges(e, _clipV) {
        leda::edge b = graph.face_cycle_succ(e); // boundary edge
        if(Color::BLACK == GetColor(graph, b)) {
            InvalidateU(graph, b);
        }
    }

    if(3 < graph.outdeg(_clipV)) SimplifyFace(graph, _clipV);

    graph.del_node(_clipV);
    _numClips++;

    if(RunMode::STEP == GetRunConf().mode) {
        graph.compute_faces();
        if(_g.hullEdges[Side::FRONT]) graph.set_visible(graph.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) graph.set_visible(graph.face_of(_g.hullEdges[Side::BACK]), false);
        ApplyEdgeColors(graph);
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