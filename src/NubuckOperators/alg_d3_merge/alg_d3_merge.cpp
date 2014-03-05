#include <Nubuck\operators\standard_algorithm.h>
#include "shared.h"

namespace OP {
namespace ALG {

struct Globals {
    Nubuck nb;

    IGeometry* geom0;
    IGeometry* geom1;
    IGeometry* geom; // union of geom0, geom1

    IGeometry* geom_suppEdge;
    IGeometry* geom_activeEdge0;
    IGeometry* geom_activeEdge1;

    Conf0 P0;
    Conf1 P1;
} g;

struct Phase0 : Phase {
    IGeometry* CreateHighlightedEdgeGeometry(const point_t& p0, const point_t& p1) {
		IGeometry* geom = g.nb.world->CreateGeometry();
		geom->SetRenderMode(IGeometry::RenderMode::EDGES);
		mesh_t& mesh = geom->GetRatPolyMesh();
		leda::node v0 = mesh.new_node(), v1 = mesh.new_node();
		mesh.set_position(v0, p0);
		mesh.set_position(v1, p1);
		mesh.set_reversal(mesh.new_edge(v0, v1), mesh.new_edge(v1, v0));
		leda::edge e = mesh.first_adj_edge(v0);
		mesh.set_color(e, R::Color::Red);
		mesh.set_radius(e, 0.05f);
		geom->Update();
        return geom;
	}

    void Enter() override {
        std::vector<IGeometry*> selection = g.nb.world->GetSelection()->GetList();
        if(2 != selection.size()) {
            printf("ERROR: invalid selection...\n");
            return;
        }

        printf("Selecting stuff...\n");
        g.geom0 = selection[0];
        g.geom1 = selection[1];

        printf("STEPPING...\n");
        Step();
    }

    void Step() {
        hull2_t H0, H1;
        leda::list_item maxH0, minH1;

        mesh_t& G0 = g.geom0->GetRatPolyMesh();
        mesh_t& G1 = g.geom1->GetRatPolyMesh();

        ConvexHullXY_Graham(G0, H0, NULL, &maxH0, true);
        ConvexHullXY_Graham(G1, H1, &minH1, NULL, true);

        const int renderAll =
            IGeometry::RenderMode::NODES |
            IGeometry::RenderMode::EDGES |
            IGeometry::RenderMode::FACES;

        g.geom = g.nb.world->CreateGeometry();
        g.geom->SetRenderMode(renderAll);
        mesh_t& G = g.geom->GetRatPolyMesh();
        G.join(G0);
        G.join(G1);
        g.geom->Update();

        g.nb.world->GetSelection()->Clear();
        g.geom0->Destroy();
        g.geom1->Destroy();

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
    }
};

class D3_Merge : public StandardAlgorithm {
protected:
    const char* GetName() const override;
    Phase*      Init(const Nubuck& nb) override;
};

const char* D3_Merge::GetName() const {
    return "Merge";
}

Phase* D3_Merge::Init(const Nubuck& nb) {
    g.nb = nb;
    return new Phase0;
}

} // namespace OP
} // namespace ALG

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new OP::ALG::StandardAlgorithmPanel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new OP::ALG::D3_Merge;
}