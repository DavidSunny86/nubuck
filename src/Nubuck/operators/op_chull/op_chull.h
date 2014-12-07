#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

namespace OP {

class ConvexHullPanel : public OperatorPanel {
public:
	ConvexHullPanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class ConvexHull : public Operator {
private:
    typedef leda::d3_rat_point point3_t;
public:
    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP
