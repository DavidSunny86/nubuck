#pragma once

#include <generic\uncopyable.h>
#include <renderer\renderer.h>
#include <renderer\mesh\meshmgr_fwd.h>
#include "r_edges.h"

namespace R {

class CylinderEdges : private GEN::Uncopyable, public EdgeRenderer {
private:
    std::vector<Edge>   _edges;
    meshPtr_t           _mesh;

    void DestroyMesh();
public:
    CylinderEdges() : _mesh(NULL) { }
    ~CylinderEdges();

    bool IsEmpty() const override { return _edges.empty(); }

    void Clear() override;
    void Push(const Edge& edge) override { _edges.push_back(edge); }
    void Rebuild() override;

    void Transform(const M::Matrix4&) override { }

    MeshJob GetRenderJob() const override;
};

} // namespace R