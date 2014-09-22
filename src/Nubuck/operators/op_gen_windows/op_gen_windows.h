#pragma once

#include <Nubuck\operators\operator.h>

namespace OP {
namespace GEN {

class WindowsPanel : public OperatorPanel {
public:
    WindowsPanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class Windows : public Operator {
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace GEN
} // namespace OP