#pragma once

#include <Nubuck\operators\operator.h>

namespace OP {
namespace GEN {

class WindowsPanel : public OperatorPanel {
public:
    WindowsPanel() { }
};

class Windows : public Operator {
public:
    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace GEN
} // namespace OP