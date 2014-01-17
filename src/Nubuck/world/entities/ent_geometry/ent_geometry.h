#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include <world\entity.h>

namespace W {

class ENT_Geometry : public IGeometry, public Entity {
private:
    leda::nb::RatPolyMesh _ratPolyMesh;

    std::vector<R::Mesh::Vertex>    _vertices;
    std::vector<R::Mesh::Vertex>    _tfverts;
    std::vector<R::Mesh::Index>     _indices;
    R::Mesh::Desc                   _meshDesc;
    R::meshPtr_t                    _mesh;
    bool                            _meshCompiled; // TODO: might be a race cond

    int                 _renderMode;
    ShadingMode::Enum   _shadingMode;
    R::RenderList       _renderList;

    void TransformVertices();
public:
    ENT_Geometry();

    bool IsMeshCompiled() const { return _meshCompiled; }
    void CompileMesh();

    void Destroy() override { Entity::Destroy(); }

    leda::nb::RatPolyMesh& GetRatPolyMesh() override { return _ratPolyMesh; }
    void Update() override;

    void SetPosition(float x, float y, float z) override { GetTransform().position = M::Vector3(x, y, z); }
    void Rotate(float ang, float x, float y, float z) override;

    void SetRenderMode(int flags) override { _renderMode = flags; }
    void SetShadingMode(ShadingMode::Enum mode) override { _shadingMode = mode; }

    void BuildRenderList();
    const R::RenderList& GetRenderList() const { return _renderList; }
};

} // namespace W