#include "phase2.h"

static bool IsUndirectedPurple(const leda::edge e) {
    return 
        PURPLE == g.edgeColors[e] ||
        PURPLE == g.edgeColors[GetG().reversal(e)];
}

void Phase2::Push(const leda::edge e) {
    if(YELLOW != g.edgeColors[e] && PURPLE != g.nodeColors[leda::source(e)])
        dfs.push(e);
}

void Phase2::Enter() {
    std::cout << "entering Phase2" << std::endl;

    leda::edge e;
    forall_edges(e, GetG()) {
        // don't push undirected purple edges
        if(RED == g.edgeColors[e] && !IsUndirectedPurple(e))
            dfs.push(e);
    }
}

Phase2::StepRet::Enum Phase2::Step() {
    mesh_t& G = GetG();

    if(dfs.empty()) return StepRet::DONE;

    leda::edge e = dfs.top();
    dfs.pop();

    if(!IsUndirectedPurple(e)) {
        g.edgeColors[e] = YELLOW;
        g.edgeColors[G.reversal(e)] = YELLOW;

        G.set_color(e, R::Color::White);

        g.nodeColors[G.target(e)] = M::Max(RED, g.nodeColors[G.target(e)]);

        if(RED == g.nodeColors[G.target(e)]) 
            G.set_color(G.target(e), R::Color::Red);
    }

    Push(G.cyclic_adj_pred(e));
    Push(G.cyclic_adj_succ(e));
    Push(G.face_cycle_succ(e));

    g.geom->Update();

    return StepRet::CONTINUE;
}