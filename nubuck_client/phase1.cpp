#include "phase1.h"

namespace {

    void BuildTetrahedron(graph_t& G, leda::node v0, leda::node v1, leda::node v2, leda::node v3) {
        G.del_all_edges();

        if(0 < leda::orientation(G[v0], G[v1], G[v2], G[v3])) {
            std::swap(v2, v3);
        }

        leda::edge e01 = G.new_edge(v0, v1);
        leda::edge e02 = G.new_edge(v0, v2);
        leda::edge e03 = G.new_edge(v0, v3);

        leda::edge e10 = G.new_edge(v1, v0);
        leda::edge e13 = G.new_edge(v1, v3);
        leda::edge e12 = G.new_edge(v1, v2);
    
        leda::edge e21 = G.new_edge(v2, v1);
        leda::edge e23 = G.new_edge(v2, v3);
        leda::edge e20 = G.new_edge(v2, v0);

        leda::edge e30 = G.new_edge(v3, v0);
        leda::edge e32 = G.new_edge(v3, v2);
        leda::edge e31 = G.new_edge(v3, v1);
    
        G.set_reversal(e01, e10);
        G.set_reversal(e02, e20);
        G.set_reversal(e03, e30);
        G.set_reversal(e13, e31);
        G.set_reversal(e12, e21);
        G.set_reversal(e23, e32);
    }

} // unnamed namespace

void Phase1::Enter(void) {
    g.nb.log->printf("--- entering Phase1.\n");
}

void Phase1::Leave(void) {
    g.nb.log->printf("--- leaving Phase1.\n");
}

IPhase::StepRet Phase1::Step(void) {
    BuildTetrahedron(g.G, g.tVerts[0], g.tVerts[1], g.tVerts[2], g.tVerts[3]);
    g.polyhedron->Update();

    // reset node colors to white
    leda::node n;
    forall_nodes(n, g.G) {
        g.polyhedron->SetNodeColor(n, 1.0f, 1.0f, 1.0f);
    }

    return CONTINUE;
}

IPhase* Phase1::NextPhase(void) {
    return NULL;
}