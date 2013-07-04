#include "phase2.h"

namespace {

    bool IsValidEdge(const graph_t& G, leda::edge e0) {
        const leda::edge e1 = G.reversal(e0);

        const point_t& p0 = G[G.source(e0)];
        const point_t& p1 = G[G.target(e0)];
        const point_t& w0 = G[G.target(G.cyclic_adj_pred(e1))];
        const point_t& w1 = G[G.target(G.cyclic_adj_pred(e0))];
            
        return 0 != leda::orientation(p0, p1, w0, w1);
    }

} // unnamed namespace

leda::edge Phase2::NextUnvisitedEdge(void) const {
    leda::edge e;
    forall_edges(e, g.G) {
        if(NOT_VISITED == edges[e]) return e;
    }
    return NULL;
}

void Phase2::MarkFace(leda::edge e, FaceState state) {
    leda::edge it = e;
    do {
        edges[it] = state;
        it = g.G.face_cycle_succ(it);
    } while(e != it);
}

bool Phase2::IsVisible(leda::node n, leda::edge e) const {
    leda::edge it = g.G.face_cycle_succ(e);

    const point_t& p = g.G[source(e)];
    const point_t& q = g.G[target(e)];
    const point_t& r = g.G[target(it)];

    return 0 < leda::orientation(p, q, r, g.G[n]);
}

leda::edge Phase2::GetHorizontEdge(void) const {
    leda::edge e;
    forall_edges(e, g.G) {
        if(VISIBLE == edges[e] && INVISIBLE == edges[g.G.reversal(e)])
            return e;
    }
    return NULL;
}

leda::edge Phase2::NextHorizontEdge(leda::edge e) const {
    leda::edge it;
    forall_out_edges(it, target(e)) {
        if(VISIBLE == edges[it] && INVISIBLE == edges[g.G.reversal(it)]) return it;
    }
    return NULL;
}

void Phase2::Enter(void) {
    g.nb.log->printf("--- entering Phase2.\n");

    curNode = g.L.pop_back();
    g.polyhedron->SetNodeColor(curNode, 1.0f, 0.0f, 0.0f);

    edges.init(g.G, NOT_VISITED);
}

void Phase2::Leave(void) {
    g.nb.log->printf("--- leaving Phase2.\n");
}

IPhase::StepRet Phase2::Step(void) {
    leda::edge e;

    if(e = NextUnvisitedEdge()) {
        g.nb.log->printf("face (e.id = %d), ", e->id());

        if(IsVisible(curNode, e)) {
            g.nb.log->printf("visible   (red)\n");
            g.polyhedron->SetFaceColor(e, 1.0f, 0.0f, 0.0f);
            MarkFace(e, VISIBLE);
        } else {
            g.nb.log->printf("invisible (blue)\n");
            g.polyhedron->SetFaceColor(e, 0.0f, 0.0f, 1.0f);
            MarkFace(e, INVISIBLE);
        }
    } else {
        g.nb.log->printf("visited all faces, constructing new hull\n");

        leda::edge p = NULL;
        e = GetHorizontEdge();

        if(e) {

            leda::edge it = e;
            do {
                leda::edge e0;
                if(!p) {
                    e0 = g.G.new_edge(curNode, source(it));
                } else {
                    e0 = g.G.new_edge(p, source(it), 0, leda::behind);
                }
                p = e0;
                leda::edge e1 = g.G.new_edge(it, curNode, 0, leda::behind);
                g.G.set_reversal(e0, e1);
                edges[e0] = edges[e1] = INVISIBLE;

                edges[it] = INVISIBLE;

                if(!IsValidEdge(g.G, it)) {
                    g.G.del_edge(g.G.reversal(it));
                    g.G.del_edge(it);
                }

                it = NextHorizontEdge(it);
            } while(it);

            it = g.G.first_edge();
            while(it) {
                if(VISIBLE == edges[it]) {
                    leda::edge tmp = g.G.succ_edge(it);
                    g.G.del_edge(it);
                    it = tmp;
                } else {
                    it = g.G.succ_edge(it);
                }
            }
        } // if e

        g.polyhedron->Update();

        g.polyhedron->SetNodeColor(curNode, 0.2f, 0.2f, 0.2f);
        if(g.L.empty()) return DONE;
        curNode = g.L.pop_back();
        g.polyhedron->SetNodeColor(curNode, 1.0f, 0.0f, 0.0f);
        
        forall_edges(e, g.G) edges[e] = NOT_VISITED;
    }

    return CONTINUE;
}

IPhase* Phase2::NextPhase(void) {
    return NULL;
}