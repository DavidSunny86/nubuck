#include "phase1_level0.h"
#include "phase0.h"

nb::geometry Phase0::CreateHighlightedEdgeGeometry(const point_t& p0, const point_t& p1) {
    nb::geometry geom = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(geom, Nubuck::RenderMode::EDGES);
    nubuck().hide_geometry_outline(geom);
    mesh_t& mesh = nubuck().poly_mesh(geom);
    leda::node v0 = mesh.new_node(), v1 = mesh.new_node();
    mesh.set_position(v0, p0);
    mesh.set_position(v1, p1);
    mesh.set_reversal(mesh.new_edge(v0, v1), mesh.new_edge(v1, v0));
    leda::edge e = mesh.first_adj_edge(v0);
    mesh.set_color(e, R::Color::Red);
    mesh.set_radius(e, 0.05f);
    return geom;
}

void Phase0::Enter() {
    std::vector<nb::geometry> selection = nubuck().selected_geometry();
    if(2 != selection.size()) {
        printf("ERROR: invalid selection...\n");
        return;
    }
    g.geom0 = selection[0];
    g.geom1 = selection[1];

    nubuck().set_geometry_name(g.geom0, std::string("(L) " + nubuck().geometry_name(g.geom0)));
    nubuck().set_geometry_name(g.geom1, std::string("(R) " + nubuck().geometry_name(g.geom1)));
}

Phase0::StepRet::Enum Phase0::Step() {
    hull2_t H0, H1;
    leda::list_item maxH0, minH1;

    mesh_t& G0 = nubuck().poly_mesh(g.geom0);
    mesh_t& G1 = nubuck().poly_mesh(g.geom1);

    nubuck().apply_geometry_transformation(g.geom0);
    nubuck().apply_geometry_transformation(g.geom1);

    ConvexHullXY_Graham(G0, H0, NULL, &maxH0, true);
    ConvexHullXY_Graham(G1, H1, &minH1, NULL, true);

    const int renderAll =
        Nubuck::RenderMode::NODES |
        Nubuck::RenderMode::EDGES |
        Nubuck::RenderMode::FACES;

    g.geom = nubuck().create_geometry();
    nubuck().set_geometry_name(g.geom, "Hull");
    nubuck().set_geometry_render_mode(g.geom, renderAll);
    mesh_t& G = nubuck().poly_mesh(g.geom);
    G.join(G0);
    G.join(G1);

	g.edgeColors.init(G, BLUE);
    g.nodeColors.init(G, BLUE);
    g.purpleEdges.init(G, NULL);

    nubuck().clear_selection();
    nubuck().destroy_geometry(g.geom0);
    nubuck().destroy_geometry(g.geom1);

    SuppEdgeXY(G, H0, H1,
        maxH0, minH1, g.P0.term, g.P1.term);

    g.P0.first = g.P0.e = G.first_adj_edge(g.P0.term);
    g.P1.first = g.P1.e = G.first_adj_edge(g.P1.term);

    g.geom_suppEdge = CreateHighlightedEdgeGeometry(G.position_of(g.P0.term), G.position_of(g.P1.term));

    g.geom_activeEdge0 = CreateHighlightedEdgeGeometry(
        G.position_of(leda::source(g.P0.e)),
        G.position_of(leda::target(g.P0.e)));

    g.geom_activeEdge1 = CreateHighlightedEdgeGeometry(
        G.position_of(leda::source(g.P1.e)),
        G.position_of(leda::target(g.P1.e)));

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase0::NextPhase() {
	return g.phase1_level0P0;
}