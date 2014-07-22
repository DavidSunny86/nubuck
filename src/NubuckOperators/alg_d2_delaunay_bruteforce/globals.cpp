#include <Nubuck\math\math.h>
#include <Nubuck\renderer\color\color.h>
#include <Nubuck\polymesh.h>
#include "globals.h"

Color::Enum GetColor(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    return static_cast<Color::Enum>(mesh[e]);
}

void SetColorU(leda::nb::RatPolyMesh& mesh, leda::edge e, Color::Enum color) {
    mesh[e] = mesh[mesh.reversal(e)] = color;
}

void ApplyEdgeColors(leda::nb::RatPolyMesh& mesh) {
    R::Color colors[2];
    colors[Color::BLACK]    = R::Color::Black;
    colors[Color::BLUE]     = R::Color::Blue;

    leda::node_array<Color::Enum> nodeColors(mesh, Color::BLACK);

    leda::edge e;
    forall_edges(e, mesh) {
        // mesh.set_color(e, colors[M::Max(GetColor(mesh, e), GetColor(mesh, mesh.reversal(e)))]);
        assert(GetColor(mesh, e) == GetColor(mesh, mesh.reversal(e)));
        mesh.set_color(e, colors[GetColor(mesh, e)]);
        nodeColors[leda::source(e)] = M::Max(nodeColors[leda::source(e)], GetColor(mesh, e));
        nodeColors[leda::target(e)] = M::Max(nodeColors[leda::target(e)], GetColor(mesh, e));
    }

    leda::node v;
    forall_nodes(v, mesh) {
        mesh.set_color(v, colors[nodeColors[v]]);
    }
}