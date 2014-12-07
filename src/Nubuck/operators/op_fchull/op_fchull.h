#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

namespace OP {

class FlipClipPanel : public OperatorPanel {
public:
    FlipClipPanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class FlipClip : public Operator {
public:
    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP