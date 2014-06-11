#pragma once

#include <Nubuck\operators\operator.h>
#include <operators\operators.h>

namespace OP {

class JoinPanel : public OperatorPanel {
public:
	JoinPanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class Join : public Operator {
private:
    Nubuck _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP