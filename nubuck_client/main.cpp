#include <LEDA\geo\d3_hull.h>

#include <Nubuck\nubuck.h>
#include "globals.h"
#include "phase0.h"

leda::list<point_t> ToPointList(const graph_t& G) {
    leda::list<point_t> L;
    leda::node n;
    forall_nodes(n, G) L.push_back(G[n]);
    return L;
}

class Algorithm : public IAlgorithm {
private:
    Globals _globals;
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t& G) override {
        // we are expected to copy these parameters
        _globals.nb = nubuck;
        _globals.G  = G;

        // create the graphical representation of the graph
        _globals.polyhedron = _globals.nb.world->CreatePolyhedron(_globals.G);

        // we return the phase we like to start
        return new Phase0(_globals);
    }

    bool Run(void) override {
        _globals.nb.log->printf("Run: using LEDA's CONVEX_HULL algorithm.\n");
        _globals.polyhedron->Destroy();
        leda::list<point_t> L(ToPointList(_globals.G));
        _globals.G.clear();
        leda::CONVEX_HULL(L, _globals.G);
        _globals.polyhedron = _globals.nb.world->CreatePolyhedron(_globals.G);
        _globals.polyhedron->Update();
        return true;
    }
};

int main(int argc, char *argv[])
{
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}
