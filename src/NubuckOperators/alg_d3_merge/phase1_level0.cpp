#include "phase1_level0.h"

static bool IsLevel0WinnerValid(Conf& conf) {
    mesh_t& G = GetG();

    if(leda::collinear(
		G[G.source(g.P0.e)],
        G[G.source(g.P1.e)],
        G[G.target(conf.e)]))
    {
        // FAILED, points are colinear
        return false;
    }

    leda::edge adj;
    forall_adj_edges(adj, G.source(conf.e)) {
        while(VIVID_VIOLET == g.edgeColors[adj])
            adj = conf.Next(G, adj);

        if(InHPos(G.target(conf.e), G.target(adj))) {
            // "FAILED, found point in hpos" 
            return false;
        }
    }
    return true;
}

enum EdgeInfo {
    VALID,      // edge is feasible level0 winner
    COLLINEAR,  // edge is collinear with advancing edge
    NONSUPP     // not all vertices are in the neg. halfspace
                // defined by plane containing this edge
                // and the advancing edge
};

static EdgeInfo GetEdgeInfo(Conf& conf, const leda::edge e) {
    const mesh_t& G = GetG();

    if(G.source(e) == G.target(e)) {
        // size of hull is 1
        return VALID;
    }

    if(leda::collinear(
		G[G.source(g.P0.e)], 
		G[G.source(g.P1.e)], 
		G[G.target(e)]))
	{
        return COLLINEAR;
	}

    leda::edge adj;
    forall_adj_edges(adj, G.source(e)) {
        while(VIVID_VIOLET == g.edgeColors[adj])
            adj = conf.Next(G, adj);

        if(InHPos(G.target(e), G.target(adj)))
            return NONSUPP;
    }
    return VALID;
}

Phase1_Level0::Phase1_Level0(Conf& conf) : conf(conf) {
	nextPhase = Phase::NextPhase(); // default idle phase
}

void Phase1_Level0::SetNextPhase(const GEN::Pointer<Phase>& phase) { 
	nextPhase = phase; 
}

void Phase1_Level0::Enter() {
	std::cout << "Entering Phase1_Level0 on " <<  conf.Name() << std::endl;
}

Phase1_Level0::StepRet::Enum Phase1_Level0::Step() {
    const mesh_t& G = GetG();

	if(!conf.it) conf.it = conf.e;

	while(VIVID_VIOLET == g.edgeColors[conf.it])
		conf.it = conf.Next(G, conf.it);

	g.edgeColors[conf.it] = M::Max(g.edgeColors[conf.it], ORANGE);
    g.activeEdge = conf.it;

    EdgeInfo info = GetEdgeInfo(conf, conf.it);
	if(VALID == info) {
		conf.e = conf.it;
        UpdateActiveEdge();
        return StepRet::DONE;
	}

    // theory guarantees that the iteration terminates eventually
	g.edgeColors[conf.it] = RED;
	conf.it = conf.Next(G, conf.it);

    UpdateActiveEdge();

    return StepRet::CONTINUE;
}