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

EdgeInfo GetEdgeInfo(leda::nb::RatPolyMesh& mesh, leda::edge e) {
    typedef leda::d3_rat_point point_t;

    EdgeInfo inf;
    inf.isBlue = inf.isConvex = inf.isFlippable = false;

    leda::edge r = mesh.reversal(e);

    if(Color::BLUE == GetColor(mesh, e)) {
        inf.isBlue = true;
    }

    leda::edge e1 = mesh.face_cycle_succ(r);
    leda::edge e3 = mesh.face_cycle_succ(e);

    const leda::node v0 = leda::source(e1);
    const leda::node v1 = leda::target(e1);
    const leda::node v2 = leda::source(e3);
    const leda::node v3 = leda::target(e3);

    const point_t p0 = mesh.position_of(v0);
    const point_t p1 = mesh.position_of(v1);
    const point_t p2 = mesh.position_of(v2);
    const point_t p3 = mesh.position_of(v3);

    const int orient = leda::orientation(p0, p1, p2, p3);

    if(0 >= orient) inf.isConvex = true;

    int orient_130 = leda::orientation_xy(p1, p3, p0);
    int orient_132 = leda::orientation_xy(p1, p3, p2);

    if( orient_130 != orient_132 && // is convex quadliteral
        (0 != orient_130 || 4 == mesh.outdeg(v0) || Color::BLUE == GetColor(mesh, e1)) &&
        (0 != orient_132 || 4 == mesh.outdeg(v2) || Color::BLUE == GetColor(mesh, e3)))
    {
        inf.isFlippable = true;
    }

    return inf;
}
