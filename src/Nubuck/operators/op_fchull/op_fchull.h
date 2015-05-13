#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

namespace OP {

class FlipClipPanel : public OperatorPanel {
public:
    FlipClipPanel() { }
};

class FlipClip : public Operator {
public:
    std::string PreferredShortcut() const override;
    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace OP