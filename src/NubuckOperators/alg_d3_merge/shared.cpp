#include "phase1_level0.h"
#include "phase1_level1.h"
#include "shared.h"

Globals g;

void InitPhases() {
	g.phase1_level0P0 = GEN::MakePtr(new Phase1_Level0(g.P0));
	g.phase1_level0P1 = GEN::MakePtr(new Phase1_Level0(g.P1));

	g.phase1_level0P0->SetNextPhase(g.phase1_level0P1);
	g.phase1_level0P1->SetNextPhase(GEN::MakePtr(new Phase1_Level1));
}

void UpdateActiveEdge() {
    const mesh_t& G = GetG();
	if(!g.geom_activeEdge) {
		g.geom_activeEdge = g.nb.world->CreateGeometry();
        g.geom_activeEdge->SetName("active edge");
		g.geom_activeEdge->SetRenderMode(IGeometry::RenderMode::EDGES);
	}
	mesh_t& mesh = g.geom_activeEdge->GetRatPolyMesh();
	mesh.clear();
    leda::node v0 = mesh.new_node(), v1 = mesh.new_node();
	mesh.set_position(v0, G.position_of(G.source(g.activeEdge)));
	mesh.set_position(v1, G.position_of(G.target(g.activeEdge)));
    mesh.set_reversal(mesh.new_edge(v0, v1), mesh.new_edge(v1, v0));
    leda::edge e = mesh.first_adj_edge(v0);
    mesh.set_color(e, R::Color::Yellow);
    mesh.set_radius(e, 0.1f);
	g.geom_activeEdge->Update();
}

// returns true if w lies in the negative halfspace of the directed
// plane (P0.v P1.v, v, w)
bool InHNeg(const leda::node v, const leda::node w) {
    const mesh_t& G = GetG();

    const point_t& v0 = G[G.source(g.P0.e)];
    const point_t& v1 = G[G.source(g.P1.e)];

    return 0 > leda::orientation(v0, v1, G[v], G[w]);
}

bool InHPos(const leda::node v, const leda::node w) {
    const mesh_t& G = GetG();

    const point_t& v0 = G[G.source(g.P0.e)];
    const point_t& v1 = G[G.source(g.P1.e)];

    return 0 < leda::orientation(v0, v1, G[v], G[w]);
}

bool Collinear(const leda::node v) {
    const mesh_t& G = GetG();
    const point_t& v0 = G[G.source(g.P0.e)];
    const point_t& v1 = G[G.source(g.P1.e)];

    return leda::collinear(v0, v1, G[v]);
}