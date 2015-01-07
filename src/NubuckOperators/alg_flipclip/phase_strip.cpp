#include <Nubuck\polymesh.h>
#include "phase_init.h"
#include "phase_flip.h"
#include "phase_clip.h"
#include "phase_stitch.h"
#include "phase_strip.h"

namespace {

void StripTetrahedrons(leda::nb::RatPolyMesh& graph, leda::node v) {
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

        if(Color::BLACK == GetColor(graph, e2) && orient_130 != orient_132 && orient_132 != 0) {
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
}

} // unnamed namespace

Phase_Strip::Phase_Strip(Globals& g) : _g(g) { }

void Phase_Strip::Enter() {
    NB::LogPrintf("entering phase 'strip'\n");

    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    _L.clear();
    leda::node v;
    forall_nodes(v, graph) {
        int bdeg = 0, rdeg = 0;

        leda::edge e;
        forall_adj_edges(e, v) {
            if(Color::BLUE == GetColor(graph, e)) bdeg++;
            if(Color::RED == GetColor(graph, e)) rdeg++;
        }

        if(0 == bdeg && 1 < rdeg) {
            _L.push(v);
        }
    }

    _needsClipping = !_L.empty();

    _numStrips = 0;
    _stepMode = StepMode::SEARCH;

    // checking some invariantes
    leda::edge e;
    forall_edges(e, graph) {
        EdgeInfo ei = GetEdgeInfo(graph, e);
        if(Color::RED == GetColor(graph, e)) {
            assert(!ei.isConvex);
        }
        if(Color::BLACK == GetColor(graph, e)) {
            assert(ei.isConvex);
        }
    }
}

Phase_Strip::StepRet::Enum Phase_Strip::StepSearch() {
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    if(_L.empty()) {
        if(RunMode::NEXT == GetRunConf().mode) {
            graph.compute_faces();
            if(_g.hullEdges[Side::FRONT]) graph.set_visible(graph.face_of(_g.hullEdges[Side::FRONT]), false);
            if(_g.hullEdges[Side::BACK]) graph.set_visible(graph.face_of(_g.hullEdges[Side::BACK]), false);
            ApplyEdgeColors(graph);
        }

        NB::LogPrintf("number of strips: %d\n", _numStrips);
        return StepRet::DONE;
    }

    leda::node v = _L.head();

    if(RunMode::STEP == GetRunConf().mode) {
        // highlight neighbourhood of vertex v
        leda::nb::set_color(graph, R::Color(1.0f, 1.0f, 1.0f, 0.2f));
        leda::edge e;
        forall_out_edges(e, v) {
            graph.set_color(graph.face_of(e), R::Color::Red);
        }
        graph.set_color(v, R::Color::Yellow);
    }

    _stepMode = StepMode::PERFORM_STRIP;
    return StepRet::CONTINUE;
}

Phase_Strip::StepRet::Enum Phase_Strip::StepPerformStrip() {
    leda::node v = _L.pop_front();

    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    StripTetrahedrons(graph, v);
    _numStrips++;

    if(RunMode::STEP == GetRunConf().mode) {
        graph.compute_faces();
        if(_g.hullEdges[Side::FRONT]) graph.set_visible(graph.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) graph.set_visible(graph.face_of(_g.hullEdges[Side::BACK]), false);
        ApplyEdgeColors(graph);
    }

    _stepMode = StepMode::SEARCH;
    return StepRet::CONTINUE;
}

Phase_Strip::StepRet::Enum Phase_Strip::Step() {
    if(StepMode::SEARCH == _stepMode) {
        return StepSearch();
    } else {
        return StepPerformStrip();
    }
}

GEN::Pointer<OP::ALG::Phase> Phase_Strip::NextPhase() {
    if(_needsClipping) {
        return GEN::MakePtr(new Phase_Clip(_g, true));
    } else {
        if(Side::FRONT == _g.side) {
            _g.side = Side::BACK;
            return GEN::MakePtr(new Phase_Init(_g));
        } else {
            return GEN::MakePtr(new Phase_Stitch(_g));
        }
    }
}