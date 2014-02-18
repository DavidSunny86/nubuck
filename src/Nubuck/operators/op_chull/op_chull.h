#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

namespace OP {

class ConvexHullPanel : public QWidget {
public:
	ConvexHullPanel(QWidget* parent = NULL) : QWidget(parent) { }
};

class ConvexHull : public Operator {
private:
    typedef leda::d3_rat_point point3_t;

    Nubuck _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace OP
