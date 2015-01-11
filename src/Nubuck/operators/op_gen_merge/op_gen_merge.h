#pragma once

#include <UI\simple_panel\simple_panel.h>

namespace OP {
namespace GEN {

class MergePanel : public OperatorPanel {
public:
    MergePanel();
};

class Merge : public Operator {
private:
    typedef leda::d3_rat_point point3_t;
public:
    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace GEN
} // namespace OP