#include <Nubuck\operators\standard_algorithm.h>

class D2_Delaunay_BruteForce : public OP::ALG::StandardAlgorithm {
protected:
    const char*     GetName() const override;
    OP::ALG::Phase* Init(const Nubuck& nb) override;
};

const char* D2_Delaunay_BruteForce::GetName() const {
    return "Delaunay Triangulation (brute force)";
}

OP::ALG::Phase* D2_Delaunay_BruteForce::Init(const Nubuck& nb) {
    return NULL;
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new OP::ALG::StandardAlgorithmPanel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new D2_Delaunay_BruteForce;
}