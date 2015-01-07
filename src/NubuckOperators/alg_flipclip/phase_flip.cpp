#include <assert.h>
#include <Nubuck\polymesh.h>
#include "phase_init.h"
#include "phase_clip.h"
#include "phase_flip.h"

Phase_Flip::Phase_Flip(Globals& g) : _g(g) { }

void Phase_Flip::Enter() {
    NB::LogPrintf("entering phase 'flip'\n");

    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    // gather red edges
    _S.clear();
    leda::edge e;
    forall_edges(e, graph) {
        if(Color::RED == GetColor(graph, e)) _S.push(e);
    }

    _numFlips = 0;
    _stepMode = StepMode::SEARCH;
}

Phase_Flip::StepRet::Enum Phase_Flip::StepSearch() {
    typedef leda::d3_rat_point point_t;

    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    while(!_S.empty()) {
        _fp.e = _S.pop();
        _fp.r = graph.reversal(_fp.e);

        if(Color::BLUE == GetColor(graph, _fp.e)) {
            continue;
        }

        _fp.e1 = graph.face_cycle_succ(_fp.r);
        _fp.e3 = graph.face_cycle_succ(_fp.e);

        const leda::node v0 = leda::source(_fp.e1);
        const leda::node v1 = leda::target(_fp.e1);
        const leda::node v2 = leda::source(_fp.e3);
        const leda::node v3 = leda::target(_fp.e3);

        const point_t p0 = graph.position_of(v0);
        const point_t p1 = graph.position_of(v1);
        const point_t p2 = graph.position_of(v2);
        const point_t p3 = graph.position_of(v3);

        const int orient = leda::orientation(p0, p1, p2, p3);

        if(0 >= orient) {
            // edge is already convex
            SetColorU(graph, _fp.e, Color::BLACK);

            if(0 == orient) {
                MarkPlanar(graph, _fp.e);
            }
        } else {
            InvalidateU(graph, _fp.e);

            _fp.orient_130 = leda::orientation_xy(p1, p3, p0);
            _fp.orient_132 = leda::orientation_xy(p1, p3, p2);

            if( _fp.orient_130 != _fp.orient_132 && // is convex quadliteral
                (0 != _fp.orient_130 || 4 == graph.outdeg(v0) || Color::BLUE == GetColor(graph, _fp.e1)) &&
                (0 != _fp.orient_132 || 4 == graph.outdeg(v2) || Color::BLUE == GetColor(graph, _fp.e3)))
            {
                // NOTE: if quadliteral is not stricly convex then it's either part of the hull
                // or a perfect diamond.

                _fp.e2 = graph.face_cycle_succ(_fp.e1);
                _fp.e4 = graph.face_cycle_succ(_fp.e3);

                _S.push(_fp.e1);
                _S.push(_fp.e2);
                _S.push(_fp.e3);
                _S.push(_fp.e4);

                if(RunMode::STEP == GetRunConf().mode) {
                    ApplyEdgeColors(graph);
                    leda::nb::set_color(graph, R::Color(1.0f, 1.0f, 1.0f, 0.2f));

                    graph.set_color(leda::source(_fp.e1), R::Color::Black);
                    graph.set_color(leda::target(_fp.e1), R::Color::Black);
                    graph.set_color(leda::source(_fp.e3), R::Color::Black);
                    graph.set_color(leda::target(_fp.e3), R::Color::Black);

                    graph.set_color(_fp.e , R::Color::Black);
                    graph.set_color(_fp.r , R::Color::Black);
                    graph.set_color(_fp.e1, R::Color::Black);
                    graph.set_color(_fp.e2, R::Color::Black);
                    graph.set_color(_fp.e3, R::Color::Black);
                    graph.set_color(_fp.e4, R::Color::Black);

                    graph.set_color(graph.face_of(_fp.e), R::Color::Red);
                    graph.set_color(graph.face_of(_fp.r), R::Color::Red);
                }

                _stepMode = StepMode::PERFORM_FLIP;
                return StepRet::CONTINUE;
            }
        }

    }

    // check invariants after flipping phase
    leda::edge e;
    forall_edges(e, graph) {
        EdgeInfo ei = GetEdgeInfo(graph, e);
        if(!ei.isBlue && !ei.isConvex) {
            assert(Color::RED == GetColor(graph, e));
            assert(!ei.isFlippable);
        }
        if(Color::RED == GetColor(graph, e)) {
            assert(!ei.isConvex);
            assert(!ei.isFlippable);
        }
        if(Color::BLACK == GetColor(graph, e)) {
            assert(ei.isConvex);
        }
    }

    // at this point S is empty
    if(RunMode::STEP == GetRunConf().mode) {
        ApplyEdgeColors(graph);
        leda::nb::set_color(graph, R::Color::White);
    } else if(RunMode::NEXT == GetRunConf().mode) {
        graph.compute_faces();
        if(_g.hullEdges[Side::FRONT]) graph.set_visible(graph.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) graph.set_visible(graph.face_of(_g.hullEdges[Side::BACK]), false);
        ApplyEdgeColors(graph);
    }

    NB::LogPrintf("number of flips: %d\n", _numFlips);
    return StepRet::DONE;
}

Phase_Flip::StepRet::Enum Phase_Flip::StepPerformFlip() {
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    const leda::node v0 = leda::source(_fp.e1);
    const leda::node v2 = leda::source(_fp.e3);

    graph.move_edge(_fp.e, _fp.e2, leda::source(_fp.e4));
    graph.move_edge(_fp.r, _fp.e4, leda::source(_fp.e2));

    if( (0 == _fp.orient_130 && Color::BLUE == GetColor(graph, _fp.e1)) ||
        (0 == _fp.orient_132 && Color::BLUE == GetColor(graph, _fp.e3)))
    {
        // quadliteral is part of hull
        SetColorU(graph, _fp.e, Color::BLUE);
    } else {
        SetColorU(graph, _fp.e, Color::BLACK);
    }

    _numFlips++;

    if(RunMode::STEP == GetRunConf().mode) {
        graph.compute_faces();
        if(_g.hullEdges[Side::FRONT]) graph.set_visible(graph.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) graph.set_visible(graph.face_of(_g.hullEdges[Side::BACK]), false);
        leda::nb::set_color(graph, R::Color(1.0f, 1.0f, 1.0f, 0.2f));

        graph.set_color(graph.face_of(_fp.e), R::Color::Blue);
        graph.set_color(graph.face_of(_fp.r), R::Color::Blue);
    }

    _stepMode = StepMode::SEARCH;

    return StepRet::CONTINUE;
}

Phase_Flip::StepRet::Enum Phase_Flip::Step() {
    if(StepMode::SEARCH == _stepMode) {
        return StepSearch();
    } else {
        return StepPerformFlip();
    }
}

GEN::Pointer<OP::ALG::Phase> Phase_Flip::NextPhase() {
    assert(_S.empty());

    return GEN::MakePtr(new Phase_Clip(_g));
}