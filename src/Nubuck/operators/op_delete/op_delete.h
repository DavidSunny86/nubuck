#pragma once

#include <Nubuck\operators\operator.h>
#include <operators\operators.h>

namespace OP {

class DeletePanel : public OperatorPanel {
public:
    DeletePanel() { }
};

class Delete : public Operator {
public:
    std::string PreferredShortcut() const override;

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace OP