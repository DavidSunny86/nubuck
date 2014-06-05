#include <Nubuck\polymesh.h>
#include <Nubuck\renderer\color\color.h>
#include "globals.h"

void ApplyEdgeColors(leda::nb::RatPolyMesh& mesh) {
    R::Color colors[3];
    colors[Color::BLACK]    = R::Color::Black;
    colors[Color::RED]      = R::Color::Red;
    colors[Color::BLUE]     = R::Color::Blue;

    leda::node_array<int> nodeColors(mesh, Color::BLACK);

    leda::edge e;
    forall_edges(e, mesh) {
        // mesh.set_color(e, colors[M::Max(mesh[e], mesh[mesh.reversal(e)])]);
        assert(mesh[e] == mesh[mesh.reversal(e)]);
        mesh.set_color(e, colors[mesh[e]]);
        nodeColors[leda::source(e)] = M::Max(nodeColors[leda::source(e)], mesh[e]);
        nodeColors[leda::target(e)] = M::Max(nodeColors[leda::target(e)], mesh[e]);
    }

    leda::node v;
    forall_nodes(v, mesh) {
        mesh.set_color(v, colors[nodeColors[v]]);
    }
}