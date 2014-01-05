#pragma once

#include <Nubuck\nubuck.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>

namespace W {

class ENT_Geometry : public IGeometry {
private:
    NB::RatPolyMesh _ratPolyMesh;

    std::vector<R::Mesh::Vertex>    _vertices;
    std::vector<R::Mesh::Index>     _indices;
    R::Mesh::Desc                   _meshDesc;
    R::MeshMgr::meshPtr_t           _mesh;
    bool                            _meshCompiled; // TODO: might be a race cond

    R::RenderList                   _renderList;
public:
    ENT_Geometry() : _mesh(NULL), _meshCompiled(true) { }

    bool IsMeshCompiled() const { return _meshCompiled; }
    void CompileMesh();

    NB::RatPolyMesh& GetRatPolyMesh() override { return _ratPolyMesh; }
    void Update() override;
    void BuildRenderList();
    const R::RenderList& GetRenderList() const { return _renderList; }
};

} // namespace W