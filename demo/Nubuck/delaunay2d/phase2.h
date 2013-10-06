#include <Nubuck\nubuck.h>
#include "globals.h"

struct Phase2 : IPhase {
    Globals& g;

    Phase2(Globals& g) : g(g) { }

    void Enter(void) override { g.nb.log->printf("--- entering Phase2.\n"); }
    void Leave(void) override { g.nb.log->printf("--- leaving Phase2.\n"); }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return false; }

    StepRet Step(void) override {
        g.nb.log->printf("Step: projecting on plane.\n");
        leda::node n;
        forall_nodes(n, g.G) {
            g.G[n] = point_t(g.G[n].X(), g.G[n].Y(), 0, g.G[n].W());
        }
        g._delaunay->Update();
        return DONE;
    }

    IPhase* NextPhase(void) override { return NULL; }
};