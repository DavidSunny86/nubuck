#pragma once

#include <Nubuck\operators\operator.h>
#include <operators\operators.h>

namespace OP {

class MergeVerticesPanel : public OperatorPanel {
public:
    MergeVerticesPanel() { }
};

class MergeVertices : public Operator {
public:
    std::string PreferredShortcut() const override;
    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace OP