#include <Nubuck\nubuck.h>
#include "globals.h"
#include "phase1.h"

struct Phase0 : IPhase {
    Globals& g;

    Phase0(Globals& g) : g(g) { }

    void Enter(void) override { g.nb.log->printf("--- entering Phase0.\n"); }
    void Leave(void) override { g.nb.log->printf("--- leaving Phase0.\n"); }

    bool IsWall(void) const override { return true; }
    bool IsDone(void) const override { return false; }

    StepRet Step(void) override {
        g.nb.log->printf("Step: projection of points on paraboloid.\n");
        leda::node n;
        forall_nodes(n, g.G) {
            const point_t& p = g.G[n];
            scalar_t z = p.xcoord() * p.xcoord() + p.ycoord() * p.ycoord();
            g._delaunay->SetNodePosition(n, point_t(p.xcoord(), p.ycoord(), z));
        }
        return DONE;
    }

    IPhase* NextPhase(void) override { return new Phase1(g); }
};