#pragma once

#include <vector>

#include <Nubuck\renderer\color\color.h>
#include <math\vector3.h>
#include <math\matrix4.h>
#include <renderer\renderer.h>

namespace R {

class EdgeRenderer {
public:
    struct Edge {
        M::Vector3  p0, p1;
        R::Color    color;
        float       radius;
    };
protected:
    static void RemoveDegeneratedEdges(std::vector<Edge>& edges);
public:
    virtual ~EdgeRenderer() { }

    virtual bool IsEmpty() const = 0;

    virtual void Clear() = 0;
    virtual void Push(const Edge& edge) = 0;
    virtual void Rebuild() = 0;

    virtual void Transform(const M::Matrix4& modelView) = 0;

    virtual MeshJob GetRenderJob() const = 0;
};

} // namespace R