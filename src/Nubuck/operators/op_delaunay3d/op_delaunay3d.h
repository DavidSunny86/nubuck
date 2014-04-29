#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>

namespace OP {

class Delaunay3DPanel : public OperatorPanel {
public:
    Delaunay3DPanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class Delaunay3D : public Operator {
private:
    Nubuck  _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace OP