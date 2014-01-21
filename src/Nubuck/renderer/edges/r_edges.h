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
    // remove edges with zero length
    template<typename EDGE>
    void RemoveDegeneratedEdges(std::vector<EDGE>& edges) {
        unsigned i = 0;
        while(edges.size() > i) {
            if(M::AlmostEqual(0.0f, M::Distance(edges[i].p0, edges[i].p1))) {
                std::swap(edges[i], edges.back());
                edges.pop_back();
            } else ++i;
        }
    }
public:
    virtual ~EdgeRenderer() { }

    virtual bool IsEmpty() const = 0;

    virtual void Clear() = 0;
    virtual void Push(const Edge& edge) = 0;
    virtual void Rebuild() = 0;

    virtual void SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) = 0;

    virtual MeshJob GetRenderJob() const = 0;
};

} // namespace R