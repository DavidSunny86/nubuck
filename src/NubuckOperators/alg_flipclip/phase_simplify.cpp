#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>
#include "phase_simplify.h"

namespace {

bool IsCollinear(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    leda::edge r = mesh.reversal(e);

    leda::edge e1 = mesh.face_cycle_succ(r);
    leda::edge e3 = mesh.face_cycle_succ(e);

    const leda::node v0 = leda::source(e1);
    const leda::node v1 = leda::target(e1);
    const leda::node v2 = leda::source(e3);
    const leda::node v3 = leda::target(e3);

    const leda::d3_rat_point p0 = mesh.position_of(v0);
    const leda::d3_rat_point p1 = mesh.position_of(v1);
    const leda::d3_rat_point p2 = mesh.position_of(v2);
    const leda::d3_rat_point p3 = mesh.position_of(v3);

    return 0 == leda::orientation(p0, p1, p2, p3);
}

} // unnamed namespace

Phase_Simplify::Phase_Simplify(Globals& g) : _g(g) { }

void Phase_Simplify::Enter() {
    _g.nb.log->printf("entering phase 'simplify'\n");

    leda::nb::RatPolyMesh& mesh = _g.geom[Side::FRONT]->GetRatPolyMesh();

    _deg.init(mesh, 0);
    leda::node v;
    forall_nodes(v, mesh) {
        _deg[v] = mesh.outdeg(v);
    }

    _inL.init(mesh, false);
    _L.clear();
    leda::edge e;
    forall_edges(e, mesh) {
        if(!_inL[e] && IsCollinear(mesh, e)) {
            _L.push(e);
            _inL[e] = _inL[mesh.reversal(e)] = true;
            mesh.set_color(e, R::Color::Yellow);
        }
    }
}

Phase_Simplify::StepRet::Enum Phase_Simplify::Step() {
    leda::nb::RatPolyMesh& mesh = _g.geom[Side::FRONT]->GetRatPolyMesh();

    _g.nb.log->printf("removing %d collinear edges.\n", _L.size());

    leda::node v, w;
    leda::edge r, e;
    forall(e, _L) {
        r = mesh.reversal(e);

        v = leda::source(e);
        w = leda::source(r);

        _deg[v]--;
        _deg[w]--;

        mesh.del_edge(r);
        mesh.del_edge(e);

        if(0 >= _deg[v]) mesh.del_node(v);
        if(0 >= _deg[w]) mesh.del_node(w);
    }
    mesh.compute_faces();

    bool isConvex = leda::CHECK_HULL(mesh);
    _g.nb.log->printf("CHECK_HULL: ");
    _g.nb.log->printf(isConvex ? "true" : "false");
    _g.nb.log->printf("\n");

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase_Simplify::NextPhase() {
    return Phase::NextPhase();
}