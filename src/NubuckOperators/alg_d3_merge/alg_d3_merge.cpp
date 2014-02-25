#include <Nubuck\operators\standard_algorithm.h>

namespace OP {
namespace ALG {

struct Globals {
    Nubuck nb;

    IGeometry* geom0;
    IGeometry* geom1;
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