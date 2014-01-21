#pragma once

#include <math\vector2.h>

struct Nubuck;

namespace OP {

class Invoker;

class Operator {
public:
    virtual ~Operator() { }

    virtual void Register(const Nubuck& nb, Invoker& invoker) = 0;
    virtual void Invoke() = 0;
    virtual void Finish() = 0;

    virtual void OnMouseDown(const M::Vector2& mouseCoords) { }
    virtual void OnMouseMove(const M::Vector2& mouseCoords) { }
};

} // namespace OP