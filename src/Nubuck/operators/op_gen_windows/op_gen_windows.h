#pragma once

#include <Nubuck\operators\operator.h>

namespace OP {
namespace GEN {

class WindowsPanel : public OperatorPanel {
public:
    WindowsPanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class Windows : public Operator {
private:
    Nubuck _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace GEN
} // namespace OP