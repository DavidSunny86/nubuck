#include <Nubuck\nubuck.h>
#include "globals.h"
#include "phase0.h"


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
};

int main(int argc, char *argv[])
{
    return RunNubuck(argc, argv, CreateAlgorithm<Algorithm>);
}
