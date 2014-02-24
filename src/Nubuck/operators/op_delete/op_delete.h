#pragma once

#include <Nubuck\operators\operator.h>
#include <operators\operators.h>

namespace OP {

class DeletePanel : public OperatorPanel {
public:
	DeletePanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class Delete : public Operator {
private:
    Nubuck _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace OP