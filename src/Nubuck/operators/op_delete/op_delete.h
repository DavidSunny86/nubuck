#pragma once

#include <Nubuck\operators\operator.h>
#include <operators\operators.h>

namespace OP {

class DeletePanel : public OperatorPanel {
public:
	DeletePanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class Delete : public Operator {
public:
    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP