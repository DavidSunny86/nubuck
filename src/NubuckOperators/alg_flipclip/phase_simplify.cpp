#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>
#include "phase_simplify.h"

namespace {

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

// these nodes somehow mess up rendering
void DeleteInnerNodes(leda::nb::RatPolyMesh& G) {
    leda::node n;
    forall_nodes(n, G) {
        if(2 == G.outdeg(n)) {
            const leda::edge e0 = G.first_adj_edge(n);
            const leda::edge e1 = G.cyclic_adj_succ(e0);

            const leda::node p0 = G.target(e0);
            const leda::node p1 = G.target(e1);

            if(leda::collinear(G[p0], G[n], G[p1])) {
                const leda::edge r0 = G.reversal(e0);
                const leda::edge r1 = G.reversal(e1);

                G.set_reversal(
                    G.new_edge(r0, p1),
                    G.new_edge(r1, p0));

                G.del_edge(e0);
                G.del_edge(e1);
                G.del_edge(r0);
                G.del_edge(r1);
            }
        }
    }
}

void DeleteLooseNodes(leda::nb::RatPolyMesh& G) {
    leda::node n;
    forall_nodes(n, G) {
        if(!G.outdeg(n)) G.del_node(n);
    }
}

} // unamed namespace

Phase_Simplify::Phase_Simplify(Globals& g) : _g(g) { }

void Phase_Simplify::Enter() {
    NB::LogPrintf("entering phase 'simplify'\n");

    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[Side::FRONT]);

    _deg.init(graph, 0);
    leda::node v;
    forall_nodes(v, graph) {
        _deg[v] = graph.outdeg(v);
    }

    _inL.init(graph, false);
    _L.clear();
    leda::edge e;
    forall_edges(e, graph) {
        bool isPlanar =
            (Color::BLACK == GetColor(graph, e) && IsMarkedPlanar(graph, e)) ||
            IsCollinear(graph, e); // blue edges are unmarked

        if(IsCollinear(graph, e)) assert(isPlanar);

        if(!_inL[e] && isPlanar) {
            assert(IsCollinear(graph, e));
            _L.push(e);
            _inL[e] = _inL[graph.reversal(e)] = true;
            graph.set_color(e, R::Color::Yellow);
        }
    }
}

Phase_Simplify::StepRet::Enum Phase_Simplify::Step() {
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[Side::FRONT]);

    NB::LogPrintf("removing %d collinear edges.\n", _L.size());

    leda::node v, w;
    leda::edge r, e;
    forall(e, _L) {
        r = graph.reversal(e);

        graph.del_edge(r);
        graph.del_edge(e);

    }

    DeleteInnerNodes(graph);
    DeleteLooseNodes(graph);

    graph.compute_faces();

    NB::LogPrintf("CHECK_HULL: ... ");
    bool isConvex = leda::CHECK_HULL(graph);
    NB::LogPrintf(isConvex ? "true" : "false");
    NB::LogPrintf("\n");

    NB::LogPrintf("convex hull:\n");
    NB::LogPrintf("... |V| = %d\n", graph.number_of_nodes());
    NB::LogPrintf("... |E| = %d\n", graph.number_of_edges());
    NB::LogPrintf("... |F| = %d\n", graph.number_of_faces());

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase_Simplify::NextPhase() {
    return Phase::NextPhase();
}