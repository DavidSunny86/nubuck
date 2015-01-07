#include "phase1_level0.h"
#include "phase0.h"

NB::Mesh Phase0::CreateHighlightedEdgeGeometry(const point_t& p0, const point_t& p1) {
    NB::Mesh geom = NB::CreateMesh();
    NB::SetMeshRenderMode(geom, NB::RM_EDGES);
    NB::HideMeshOutline(geom);
    mesh_t& mesh = NB::GetGraph(geom);
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
    NB::Mesh sel1, sel0 = NB::FirstSelectedMesh();
    if(!sel0 || !(sel1 = NB::NextSelectedMesh(sel0))) {
        printf("ERROR: invalid selection...\n");
        return;
    }
    g.geom0 = sel0;
    g.geom1 = sel1;

    NB::SetMeshName(g.geom0, std::string("(L) " + NB::GetMeshName(g.geom0)));
    NB::SetMeshName(g.geom1, std::string("(R) " + NB::GetMeshName(g.geom1)));
}

Phase0::StepRet::Enum Phase0::Step() {
    hull2_t H0, H1;
    leda::list_item maxH0, minH1;

    mesh_t& G0 = NB::GetGraph(g.geom0);
    mesh_t& G1 = NB::GetGraph(g.geom1);

    NB::ApplyMeshTransformation(g.geom0);
    NB::ApplyMeshTransformation(g.geom1);

    ConvexHullXY_Graham(G0, H0, NULL, &maxH0, true);
    ConvexHullXY_Graham(G1, H1, &minH1, NULL, true);

    g.geom = NB::CreateMesh();
    NB::SetMeshName(g.geom, "Hull");
    NB::SetMeshRenderMode(g.geom, NB::RM_ALL);
    mesh_t& G = NB::GetGraph(g.geom);
    G.join(G0);
    G.join(G1);

	g.edgeColors.init(G, BLUE);
    g.nodeColors.init(G, BLUE);
    g.purpleEdges.init(G, NULL);

    NB::ClearSelection();
    NB::DestroyMesh(g.geom0);
    NB::DestroyMesh(g.geom1);

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