#include <Nubuck\nubuck.h>
#include "globals.h"
#include "phase0.h"

class Algorithm : public IAlgorithm {
private:
    Globals _globals;

    void Prepare(graph_t& G) {
        // project all nodes on xy-plane
        leda::node n;
        forall_nodes(n, G)
            G[n] = point_t(G[n].xcoord(), G[n].ycoord(), 0);

        // add nodes for bounding rectangle
        const leda::rational size(50); // TODO

        G[G.new_node()] = point_t(-size, -size, 0);
        G[G.new_node()] = point_t( size, -size, 0);
        G[G.new_node()] = point_t( size,  size, 0);
        G[G.new_node()] = point_t(-size,  size, 0);
    }
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t& G) override {
        // we are expected to copy these parameters
        _globals.nb = nubuck;
        _globals.G  = G;

        Prepare(_globals.G);

        _globals._delaunay = _globals.nb.world->CreatePolyhedron(_globals.G);

        return new Phase0(_globals);
    }

    bool Run(void) override { return false; /* do nothing special */ }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}