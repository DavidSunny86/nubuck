#pragma once

#include <Nubuck\operators\operator.h>

namespace OP {

struct SelectedFacesPanel : OperatorPanel {
public:
    SelectedFacesPanel() { }
};

class SelectedFaces : public Operator {
public:
    SelectedFaces();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace OP