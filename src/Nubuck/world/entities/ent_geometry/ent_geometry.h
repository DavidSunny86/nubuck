#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\math\box.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\nodes\r_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include <UI\outliner\outliner.h>
#include <world\entity.h>

namespace W {

class ENT_Geometry : public IGeometry, public Entity {
private:
    leda::nb::RatPolyMesh _ratPolyMesh;

    R::Nodes                        _nodes;
    R::CylinderEdges                _cylinderEdges;
    R::LineEdges                    _lineEdges;
    R::EdgeRenderer*                _edgeRenderer;

    void RebuildRenderNodes();
    void RebuildRenderEdges();

    std::vector<M::Vector3>         _fpos;
    std::vector<R::Mesh::Vertex>    _vertices;
    std::vector<R::Mesh::Index>     _indices;
    R::Mesh::Desc                   _meshDesc;
    R::meshPtr_t                    _mesh;
    R::tfmeshPtr_t                  _tfmesh;
    bool                            _meshCompiled; // TODO: might be a race cond

    void CacheFPos();
    void RebuildRenderMesh();

    float       _edgeRadius;
    R::Color    _edgeColor;

    bool                _isHidden;
    int                 _renderMode;
    unsigned            _renderLayer;
    ShadingMode::Enum   _shadingMode;
    R::RenderList       _renderList;

    M::Box              _bbox;      // untransformed bounding box

    void ComputeCenter();
    void ComputeBoundingBox();
public:
    ENT_Geometry();

    GEN::Pointer<UI::Outliner::View> GetOutlinerView();

    bool IsMeshCompiled() const;
    void CompileMesh();

    void Destroy() override;

    leda::nb::RatPolyMesh& GetRatPolyMesh() override;
    void Update() override;

    void ApplyTransformation();

    void SetEdgeRadius(float edgeRadius);
    void SetEdgeColor(const R::Color& color);

    const M::Vector3& GetLocalCenter() const;
    M::Vector3 GetGlobalCenter();

    const M::Box& GetBoundingBox() const;

    void GetPosition(float& x, float& y, float& z) const override;

    void SetPosition(float x, float y, float z) override;
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