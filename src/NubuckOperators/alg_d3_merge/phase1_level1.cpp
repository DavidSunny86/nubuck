#include "shared.h"
#include "mantle.h"
#include "phase2.h"
#include "phase1_level0.h"
#include "phase1_level1.h"

static void Winner(
        Conf** wConf, // winning configuration
        Conf** lConf) // losing configuration
{
    mesh_t& G = GetG();

    // initialize advancing edge if we havent hade one
    if(!g.P0.adv) { // !P0.adv iff !P1.adv
        g.P0.adv = G.new_edge(g.P0.e, G.source(g.P1.e), 0, leda::before);
        g.P1.adv = G.new_edge(g.P1.e, G.source(g.P0.e), 0, leda::behind);
        g.edgeColors[g.P0.adv] = g.edgeColors[g.P1.adv] = VIVID_VIOLET;
        G.set_reversal(g.P0.adv, g.P1.adv);
        G.set_masked(g.P0.adv);
        G.set_masked(g.P1.adv);
    }

    if(Collinear(G.target(g.P1.e)) || InHNeg(G.target(g.P0.e), G.target(g.P1.e))) 
    {
        // P0 is the winner

        *wConf = &g.P0;
        *lConf = &g.P1;

        // create new face
        g.P0.adv = G.new_edge(G.reversal(g.P0.e), G.source(g.P1.e), 0, leda::behind);
        g.P1.adv = G.new_edge(g.P1.adv, G.target(g.P0.e), 0, leda::before);
    }
    else {
        // P1 is the winner

        *wConf = &g.P1;
        *lConf = &g.P0;

        // create new face
        g.P0.adv = G.new_edge(g.P0.adv, G.target(g.P1.e), 0, leda::behind);
        g.P1.adv = G.new_edge(G.reversal(g.P1.e), G.source(g.P0.e), 0, leda::before);
    }

    g.edgeColors[g.P0.adv] = g.edgeColors[g.P1.adv] = VIVID_VIOLET;
    G.set_reversal(g.P0.adv, g.P1.adv);
    G.set_masked(g.P0.adv);
    G.set_masked(g.P1.adv);
}

// repaint adjacent orange edges blue
void Clean(leda::node v) {
    leda::edge e;
    forall_adj_edges(e, v) {
        if(ORANGE == g.edgeColors[e])
            g.edgeColors[e] = BLUE;
    }
}

Phase1_Level1::Phase1_Level1() {
	nextPhase = Phase::NextPhase(); // default idle phase
}

void Phase1_Level1::Enter() {
    isWall = false;
}

Phase1_Level1::StepRet::Enum Phase1_Level1::Step() {
    mesh_t& G = GetG();
    Conf *wConf, *lConf;
    Winner(&wConf, &lConf);

	Clean(G.source(wConf->e));

    g.edgeColors[wConf->e] = PURPLE;
    G.set_color(wConf->e, purple);
    G.set_radius(wConf->e, G.radius_of(wConf->e) * 1.5f);

    g.nodeColors[G.target(wConf->e)] = PURPLE;
    G.set_color(G.target(wConf->e), purple);

    // consider the case of one point that never wins itself, but
    // is connected to every new edge!
    // g.nodeColors[G.source(lConf->e)] = PURPLE; 

    g.purpleEdges[G.source(wConf->e)] = wConf->e;

    // construct new mantle face
	if(!g.mantle.IsValid()) g.mantle = GEN::MakePtr(new Mantle(G));
	g.mantle->AddTriangle(G.source(g.P0.e), G.source(g.P1.e), G.target(wConf->e));

    if(g.P0.term == G.target(g.P0.e)) {
        leda::edge e = G.cyclic_adj_pred(G.reversal(g.P0.e));
        while(RED == g.edgeColors[e]) {
            g.edgeColors[e] = BLUE;
            e = G.cyclic_adj_pred(e);
        }
    }

    if(g.P1.term == G.target(g.P1.e)) {
        leda::edge e = G.cyclic_adj_succ(G.reversal(g.P1.e));
        while(RED == g.edgeColors[e]) {
            g.edgeColors[e] = BLUE;
            e = G.cyclic_adj_succ(e);
        }
    }

    wConf->first = wConf->e = G.reversal(wConf->e);

    g.P0.it = g.P1.it = NULL;

    if( g.P0.term == G.source(g.P0.e) && 
        g.P1.term == G.source(g.P1.e))
    {
		nextPhase = GEN::MakePtr(new Phase2);
        isWall = true;
        // wrapping completed
    }
	else nextPhase = g.phase1_level0P0;

    return StepRet::DONE;
}