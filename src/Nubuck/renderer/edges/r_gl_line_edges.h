#pragma once

#include <vector>
#include <renderer\mesh\mesh.h>
#include "r_edges.h"

namespace R {

class GL_LineEdges : private GEN::Uncopyable, public EdgeRenderer {
private:
    std::vector<Edge>           _edges;
    std::vector<Mesh::Vertex>   _meshVertices;
    std::vector<Mesh::Index>    _meshIndices;
    Mesh::Desc                  _meshDesc;
    meshPtr_t                   _mesh;
    tfmeshPtr_t                 _tfmesh;
    bool                        _needsRebuild;
    bool                        _isInvalid;

    void DestroyMesh();
public:
    GL_LineEdges();

    bool IsEmpty() const override { return _edges.empty(); }

    void Rebuild(const std::vector<Edge>& edges) override;
    void Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) override;

    void SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) override;

    void BuildRenderMesh() override;
    void DestroyRenderMesh() override;

    MeshJob GetRenderJob() const override;
};

} // namespace R