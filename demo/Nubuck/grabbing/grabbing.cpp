#include <Nubuck\nubuck.h>
#include "globals.h"
#include "phase0.h"

class Algorithm : public IAlgorithm {
private:
    Globals _globals;

    void Prepare(graph_t& G) {
        leda::node n;

        // project all nodes on xy-plane. scale, too
        double s = 0.01;
        _globals.nb.log->printf("Scaling points by factor %f.\n", s);
        forall_nodes(n, G)
            G[n] = point_t(s * G[n].xcoord(), s * G[n].ycoord(), 0);
    }
public:
    IPhase* Init(const Nubuck& nubuck, const graph_t& G) override {
        // we are expected to copy these parameters
        _globals.nb = nubuck;
        _globals.grNodes  = G;

        Prepare(_globals.grNodes);
        Delaunay2D(_globals.grNodes, _globals.grDelaunay);

        _globals.phNodes = _globals.nb.world->CreatePolyhedron(_globals.grNodes);
        _globals.phNodes->SetRenderFlags(POLYHEDRON_RENDER_NODES);
        _globals.phNodes->SetPickable(true);

        _globals.phDelaunay = _globals.nb.world->CreatePolyhedron(_globals.grDelaunay);
        _globals.phDelaunay->SetRenderFlags(POLYHEDRON_RENDER_EDGES);

        return new Phase0(_globals);
    }

    bool Run(void) override { return false; /* do nothing special */ }
};

int main(int argc, char* argv[]) {
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}