#pragma once

#include <UI\simple_panel\simple_panel.h>

namespace OP {
namespace GEN {

class MergePanel : public OperatorPanel {
public:
    MergePanel(QWidget* parent = NULL);
};

class Merge : public Operator {
private:
    typedef leda::d3_rat_point point3_t;
    Nubuck _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace GEN
} // namespace OP