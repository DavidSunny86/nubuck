#pragma once

#include <vector>
#include <math\vector2.h>

struct Nubuck;

namespace R {

struct MeshJob;

} // namespace R

namespace OP {

class Invoker;

class Operator {
public:
    virtual ~Operator() { }

    virtual void Register(const Nubuck& nb, Invoker& invoker) = 0;
    virtual void Invoke() = 0;
    virtual void Finish() = 0;

    virtual void GetMeshJobs(std::vector<R::MeshJob>& meshJobs) { }
    virtual void OnGeometrySelected() { }
    virtual void OnCameraChanged() { }
    virtual bool OnMouseDown(const M::Vector2& mouseCoords, bool shiftKey) { return false; }
    virtual bool OnMouseUp(const M::Vector2& mouseCoords)  { return false; }
    virtual bool OnMouseMove(const M::Vector2& mouseCoords) { return false; }
};

} // namespace OP