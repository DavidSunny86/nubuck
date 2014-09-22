#include <Nubuck\operators\standard_algorithm.h>
#include "shared.h"
#include "phase0.h"

class D3_Merge : public OP::ALG::StandardAlgorithm {
protected:
    const char*         GetName() const override;
	OP::ALG::Phase*     Init() override;
};

const char* D3_Merge::GetName() const {
    return "Merge";
}

OP::ALG::Phase* D3_Merge::Init() {
    if(2 > nubuck().selected_geometry().size()) {
        nubuck().log_printf("select two objects as input.");
        return NULL;
    }

    InitPhases();
    return new Phase0;
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new OP::ALG::StandardAlgorithmPanel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new D3_Merge;
}