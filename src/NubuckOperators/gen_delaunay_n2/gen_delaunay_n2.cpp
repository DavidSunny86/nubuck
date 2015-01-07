#include <QMenu>
#include <QAction>

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>

class D3_DelaunayN2_Panel : public OP::OperatorPanel {
};

class D3_DelaunayN2 : public OP::Operator {
public:
    void Register(OP::Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

void D3_DelaunayN2::Register(OP::Invoker& invoker) {
    QAction* action = NB::SceneMenu()->addAction("Delaunay O(n^2)");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool D3_DelaunayN2::Invoke() {
    NB::SetOperatorName("Delaunay, O(n^2)");

    NB::Mesh mesh = NB::CreateMesh();
    NB::SetMeshName(mesh, "Delaunay O(n^2)");
    NB::SetMeshRenderMode(mesh, NB::RM_NODES);

    leda::nb::RatPolyMesh& graph = NB::GetGraph(mesh);

    const unsigned N = 32;
    const unsigned k = N / 2;
    const unsigned m = N - k;

    const double alpha = 2.0f * M::PI / k;
    for(unsigned i = 0; i < k; ++i) {
        const leda::node v = graph.new_node();
        const leda::rational x(cos(i * alpha));
        const leda::rational y(sin(i * alpha));
        const leda::d3_rat_point pos = leda::d3_rat_point(x, y, 0);
        graph.set_position(v, pos);
    }

    const double s = 1.0 / (m - 1.0);
    for(unsigned i = 0; i < k; ++i) {
        const leda::node v = graph.new_node();
        const leda::rational z = i * s;
        const leda::d3_rat_point pos = leda::d3_rat_point(0, 0, z);
        graph.set_position(v, pos);
    }

    return true;
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new D3_DelaunayN2_Panel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new D3_DelaunayN2;
}