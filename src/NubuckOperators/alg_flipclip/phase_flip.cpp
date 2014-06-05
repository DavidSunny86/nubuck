#include <assert.h>
#include <Nubuck\polymesh.h>
#include "phase_clip.h"
#include "phase_flip.h"

Phase_Flip::Phase_Flip(Globals& g) : _g(g) { }

void Phase_Flip::Enter() {
    _g.nb.log->printf("entering phase 'flip'\n");

    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    // gather red edges
    _S.clear();
    leda::edge e;
    forall_edges(e, mesh) {
        if(Color::RED == mesh[e]) _S.push(e);
    }

    _stepMode = StepMode::SEARCH;
}

Phase_Flip::StepRet::Enum Phase_Flip::StepSearch() {
    typedef leda::d3_rat_point point_t;

    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    while(!_S.empty()) {
        _fp.e = _S.pop();
        _fp.r = mesh.reversal(_fp.e);

        if(Color::BLUE == mesh[_fp.e]) {
            continue;
        }

        _fp.e1 = mesh.face_cycle_succ(_fp.r);
        _fp.e3 = mesh.face_cycle_succ(_fp.e);

        const leda::node v0 = leda::source(_fp.e1);
        const leda::node v1 = leda::target(_fp.e1);
        const leda::node v2 = leda::source(_fp.e3);
        const leda::node v3 = leda::target(_fp.e3);

        const point_t p0 = mesh.position_of(v0);
        const point_t p1 = mesh.position_of(v1);
        const point_t p2 = mesh.position_of(v2);
        const point_t p3 = mesh.position_of(v3);

        if(0 >= leda::orientation(p0, p1, p2, p3)) {
            // edge is already convex
            mesh[_fp.e] = mesh[_fp.r] = Color::BLACK;
        } else {
            mesh[_fp.e] = mesh[_fp.r] = Color::RED;

            _fp.orient_130 = leda::orientation_xy(p1, p3, p0);
            _fp.orient_132 = leda::orientation_xy(p1, p3, p2);

            if( _fp.orient_130 != _fp.orient_132 && // is convex quadliteral
                (0 != _fp.orient_130 || 4 == mesh.outdeg(v0) || Color::BLUE == mesh[_fp.e1]) &&
                (0 != _fp.orient_132 || 4 == mesh.outdeg(v2) || Color::BLUE == mesh[_fp.e3]))
            {
                // NOTE: if quadliteral is not stricly convex then it's either part of the hull
                // or a perfect diamond.

                _fp.e2 = mesh.face_cycle_succ(_fp.e1);
                _fp.e4 = mesh.face_cycle_succ(_fp.e3);

                _S.push(_fp.e1);
                _S.push(_fp.e2);
                _S.push(_fp.e3);
                _S.push(_fp.e4);

                ApplyEdgeColors(mesh);
                leda::nb::set_color(mesh, R::Color(1.0f, 1.0f, 1.0f, 0.2f));

                mesh.set_color(leda::source(_fp.e1), R::Color::Black);
                mesh.set_color(leda::target(_fp.e1), R::Color::Black);
                mesh.set_color(leda::source(_fp.e3), R::Color::Black);
                mesh.set_color(leda::target(_fp.e3), R::Color::Black);

                mesh.set_color(_fp.e , R::Color::Black);
                mesh.set_color(_fp.r , R::Color::Black);
                mesh.set_color(_fp.e1, R::Color::Black);
                mesh.set_color(_fp.e2, R::Color::Black);
                mesh.set_color(_fp.e3, R::Color::Black);
                mesh.set_color(_fp.e4, R::Color::Black);

                mesh.set_color(mesh.face_of(_fp.e), R::Color::Red);
                mesh.set_color(mesh.face_of(_fp.r), R::Color::Red);

                _stepMode = StepMode::PERFORM_FLIP;
                return StepRet::CONTINUE;
            }
        }

    }

    // at this point S is empty
    ApplyEdgeColors(mesh);
    leda::nb::set_color(mesh, R::Color::White);

    return StepRet::DONE;
}

Phase_Flip::StepRet::Enum Phase_Flip::StepPerformFlip() {
    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    const leda::node v0 = leda::source(_fp.e1);
    const leda::node v2 = leda::source(_fp.e3);

    mesh.move_edge(_fp.e, _fp.e2, leda::source(_fp.e4));
    mesh.move_edge(_fp.r, _fp.e4, leda::source(_fp.e2));

    if( (0 == _fp.orient_130 && Color::BLUE == mesh[_fp.e1]) ||
        (0 == _fp.orient_132 && Color::BLUE == mesh[_fp.e3]))
    {
        // quadliteral is part of hull
        mesh[_fp.e] = mesh[_fp.r] = Color::BLUE;
    } else {
        mesh[_fp.e] = mesh[_fp.r] = Color::BLACK;
    }

    mesh.compute_faces();
    mesh.set_visible(mesh.face_of(_g.hullEdge), false);
    leda::nb::set_color(mesh, R::Color(1.0f, 1.0f, 1.0f, 0.2f));

    mesh.set_color(mesh.face_of(_fp.e), R::Color::Blue);
    mesh.set_color(mesh.face_of(_fp.r), R::Color::Blue);

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
    return GEN::MakePtr(new Phase_Clip(_g));
}