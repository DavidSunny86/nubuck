#pragma once

struct Nubuck;

namespace OP {

class Invoker;

class Operator {
public:
    virtual ~Operator() { }

    virtual void Register(const Nubuck& nb, Invoker& invoker) = 0;
    virtual void Invoke() = 0;
    virtual void Finish() = 0;
};

} // namespace OP