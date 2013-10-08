#include <Nubuck\nubuck.h>
#include "globals.h"

struct Phase0 : IPhase {
    typedef leda::rat_point point2_t;
    typedef leda::GRAPH<point2_t, int> graph2_t;

    Globals& g;

    Phase0(Globals& g) : g(g) { }

    void Enter(void) override { g.nb.log->printf("--- entering Phase0.\n"); }
    void Leave(void) override { g.nb.log->printf("--- leaving Phase0.\n"); }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return false; }

    StepRet Step(void) override {
        return DONE;
    }

    IPhase* NextPhase(void) override { return NULL; }

    leda::list<point2_t> ToPointList2(const graph_t& G) {
        leda::list<point2_t> L;
        leda::node n;
        forall_nodes(n, G) L.push_back(point2_t(G[n].xcoord(), G[n].ycoord()));
        return L;
    }

    void FromProjection(const graph2_t& G2, graph_t& G3) {
        leda::node_array<leda::node> nmap(G2, NULL);

        leda::node n2;
        forall_nodes(n2, G2) {
            leda::node n3 = nmap[n2] = G3.new_node();
            G3[n3] = point_t(G2[n2].xcoord(), G2[n2].ycoord(), 0);
        }

        leda::edge e2;
        forall_edges(e2, G2) {
            G3.new_edge(nmap[leda::source(e2)], nmap[leda::target(e2)]); 
        }
    }

    void OnNodesMoved(void) override {
        leda::node n;
        forall_nodes(n, g.grNodesProj) {
            const point_t& p = g.grNodesProj[n];
            scalar_t z = 5 + (p.xcoord() * p.xcoord() + p.ycoord() * p.ycoord()) / 100;
            g.grNodes[g.nmap[n]] = point_t(p.xcoord(), p.ycoord(), z);
        }
        Delaunay2D(g.grNodesProj, g.grDelaunayProj);
        ConvexHull(g.grNodesProj, g.grHullProj);
        ConvexHull(g.grNodes, g.grHull);
        g.phDelaunayProj->Update();
        g.phHullProj->Update();
        g.phNodes->Update();
        g.phHull->Update();
    }

    void OnKeyPressed(char c) override {
        if('S' == c) {
            if(g.showHull) {
                g.phNodes->SetRenderFlags(0);
                g.phHull->SetRenderFlags(0);
                g.showHull = false;
            } else {
                g.phNodes->SetRenderFlags(POLYHEDRON_RENDER_NODES);
                g.phHull->SetRenderFlags(POLYHEDRON_RENDER_EDGES | POLYHEDRON_RENDER_HULL);
                g.phNodes->Update();
                g.phHull->Update();
                g.showHull = true;
            }
        }
    }
};