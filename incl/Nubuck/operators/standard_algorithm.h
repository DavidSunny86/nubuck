#pragma once

#include <string>

#include <Nubuck\operators\operator.h>

namespace OP {
namespace ALG {

class NUBUCK_API StandardAlgorithm : public Operator {
private:
    Nubuck _nb;
protected:
    virtual const std::string& GetName() const = 0;
public:
    void Register(const Nubuck& nb, Invoker& invoker) override;

    void Invoke() override;
    void Finish() override { }
};

} // namespace ALG
} // namespace OP