#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\nodes\r_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include <world\entity.h>

namespace W {

class ENT_Geometry : public IGeometry, public Entity {
private:
    leda::nb::RatPolyMesh _ratPolyMesh;

    R::Nodes                        _nodes;
    R::CylinderEdges                _cylinderEdges;
    R::LineEdges                    _lineEdges;
    R::EdgeRenderer*                _edgeRenderer;

    std::vector<R::Mesh::Vertex>    _vertices;
    std::vector<R::Mesh::Vertex>    _tfverts;
    std::vector<R::Mesh::Index>     _indices;
    R::Mesh::Desc                   _meshDesc;
    R::meshPtr_t                    _mesh;
    bool                            _meshCompiled; // TODO: might be a race cond

    bool                _isHidden;
    int                 _renderMode;
    unsigned            _renderLayer;
    ShadingMode::Enum   _shadingMode;
    R::RenderList       _renderList;

    M::Vector3          _center; // untransformed center

    M::Matrix4          _M;

    void TransformVertices();
    void ComputeCenter();
public:
    ENT_Geometry();

    void SetM(const M::Matrix4& M) { _M = M; }

    bool IsMeshCompiled() const { return _meshCompiled; }
    void CompileMesh();

    void Destroy() override { Entity::Destroy(); }

    leda::nb::RatPolyMesh& GetRatPolyMesh() override { return _ratPolyMesh; }
    void Update() override;

    const M::Vector3& GetLocalCenter() const { return _center; }
    M::Vector3 GetGlobalCenter() { return Transform(_center); }

    float GetPosX() const override { return GetTransform().position.x; }
    float GetPosY() const override { return GetTransform().position.y; }
    float GetPosZ() const override { return GetTransform().position.z; }

    void SetPosition(float x, float y, float z) override { GetTransform().position = M::Vector3(x, y, z); }
    void Rotate(float ang, float x, float y, float z) override;

    void Show() override { _isHidden = false; }
    void Hide() override { _isHidden = true; }

    void SetRenderMode(int flags) override { _renderMode = flags; }
    void SetRenderLayer(unsigned layer) override { _renderLayer = layer; }
    void SetShadingMode(ShadingMode::Enum mode) override { _shadingMode = mode; }

    void FrameUpdate();
    void BuildRenderList();
    const R::RenderList& GetRenderList() const { return _renderList; }
};

} // namespace W