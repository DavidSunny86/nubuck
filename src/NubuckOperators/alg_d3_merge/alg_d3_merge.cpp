#include <Nubuck\operators\standard_algorithm.h>
#include "shared.h"

namespace OP {
namespace ALG {

struct Globals {
    Nubuck nb;

    IGeometry* geom0;
    IGeometry* geom1;
    IGeometry* geom; // union of geom0, geom1
} g;

struct Phase0 : Phase {
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