#pragma once

#include <vector>
#include <generic\uncopyable.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr_fwd.h>
#include "r_edges.h"

namespace R {

class LineEdges : private GEN::Uncopyable, public EdgeRenderer {
private:
    struct Billboard { Mesh::Vertex verts[4]; };

    std::vector<Edge>       _edges;
    std::vector<Billboard>  _edgeBBoards;
    meshPtr_t               _mesh;

    void DestroyMesh();
public:
    LineEdges() : _mesh(NULL) { }
    ~LineEdges();

    bool IsEmpty() const override { return _edges.empty(); }

    void Clear() override;
    void Push(const Edge& edge) override { _edges.push_back(edge); }
    void Rebuild() override;

    void SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) override;

    MeshJob GetRenderJob() const override;
};

} // namespace R
