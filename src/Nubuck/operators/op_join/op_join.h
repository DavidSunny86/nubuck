#pragma once

#include <operators\operator.h>
#include <operators\operators.h>

namespace OP {

class Join : public Operator {
private:
    Nubuck _nb;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace OP