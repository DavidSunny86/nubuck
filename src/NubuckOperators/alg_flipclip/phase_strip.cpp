#include <Nubuck\polymesh.h>
#include "phase_init.h"
#include "phase_flip.h"
#include "phase_clip.h"
#include "phase_stitch.h"
#include "phase_strip.h"

namespace {

void StripTetrahedrons(leda::nb::RatPolyMesh& mesh, leda::node v) {
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

        if(Color::BLACK == mesh[e2] && orient_130 != orient_132 && orient_132 != 0) {
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

Phase_Strip::Phase_Strip(Globals& g) : _g(g) { }

void Phase_Strip::Enter() {
    _g.nb.log->printf("entering phase 'strip'\n");

    leda::nb::RatPolyMesh& mesh = _g.geom[_g.side]->GetRatPolyMesh();

    _L.clear();
    leda::node v;
    forall_nodes(v, mesh) {
        int bdeg = 0, rdeg = 0;

        leda::edge e;
        forall_adj_edges(e, v) {
            if(Color::BLUE == mesh[e]) bdeg++;
            if(Color::RED == mesh[e]) rdeg++;
        }

        if(0 == bdeg && 1 < rdeg) {
            assert(3 > rdeg); // clip precedes strip
            _L.push(v);
        }
    }

    _needsClipping = !_L.empty();

    _numStrips = 0;
    _stepMode = StepMode::SEARCH;
}

Phase_Strip::StepRet::Enum Phase_Strip::StepSearch() {
    leda::nb::RatPolyMesh& mesh = _g.geom[_g.side]->GetRatPolyMesh();

    if(_L.empty()) {
        if(RunMode::NEXT == GetRunConf().mode) {
            mesh.compute_faces();
            if(_g.hullEdges[Side::FRONT]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::FRONT]), false);
            if(_g.hullEdges[Side::BACK]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::BACK]), false);
            ApplyEdgeColors(mesh);
        }

        _g.nb.log->printf("number of strips: %d\n", _numStrips);
        return StepRet::DONE;
    }

    leda::node v = _L.head();

    if(RunMode::STEP == GetRunConf().mode) {
        // highlight neighbourhood of vertex v
        leda::nb::set_color(mesh, R::Color(1.0f, 1.0f, 1.0f, 0.2f));
        leda::edge e;
        forall_out_edges(e, v) {
            mesh.set_color(mesh.face_of(e), R::Color::Red);
        }
        mesh.set_color(v, R::Color::Yellow);
    }

    _stepMode = StepMode::PERFORM_STRIP;
    return StepRet::CONTINUE;
}

Phase_Strip::StepRet::Enum Phase_Strip::StepPerformStrip() {
    leda::node v = _L.pop_front();

    leda::nb::RatPolyMesh& mesh = _g.geom[_g.side]->GetRatPolyMesh();

    StripTetrahedrons(mesh, v);
    _numStrips++;

    if(RunMode::STEP == GetRunConf().mode) {
        mesh.compute_faces();
        if(_g.hullEdges[Side::FRONT]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::FRONT]), false);
        if(_g.hullEdges[Side::BACK]) mesh.set_visible(mesh.face_of(_g.hullEdges[Side::BACK]), false);
        ApplyEdgeColors(mesh);
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