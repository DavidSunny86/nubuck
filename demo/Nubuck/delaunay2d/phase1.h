#include <LEDA\geo\d3_hull.h>
#include <Nubuck\nubuck.h>
#include "globals.h"
#include "phase2.h"

struct Phase1 : IPhase {
    Globals& g;

    Phase1(Globals& g) : g(g) { }

    void Enter(void) override { g.nb.log->printf("--- entering Phase1.\n"); }
    void Leave(void) override { g.nb.log->printf("--- leaving Phase1.\n"); }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return false; }

    leda::list<point_t> ToPointList(const graph_t& G) {
        leda::list<point_t> L;
        leda::node n;
        forall_nodes(n, G) L.push_back(G[n]);
        return L;
    }

    StepRet Step(void) override {
        g.nb.log->printf("Step: computing convex hull.\n");
        leda::list<point_t> L(ToPointList(g.G));
        g.G.clear();
        leda::CONVEX_HULL(L, g.G);
        g._delaunay->Destroy();
        g._delaunay = g.nb.world->CreatePolyhedron(g.G);
        g._delaunay->Update();
        return DONE;
    }

    IPhase* NextPhase(void) override { return new Phase2(g); }
};