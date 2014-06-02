#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\math\box.h>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\world\editmode.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\nodes\r_billboard_nodes.h>
#include <renderer\nodes\r_point_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include <renderer\edges\r_gl_line_edges.h>
#include <UI\outliner\outliner.h>
#include <world\entity.h>

namespace W {

class ENT_Geometry : public IGeometry, public Entity, public EV::EventHandler<> {
private:
    leda::nb::RatPolyMesh _ratPolyMesh;

	mutable SYS::SpinLock _mtx;

    UI::Outliner::itemHandle_t      _outlinerItem;

    R::BillboardNodes               _billboardNodes;
    R::PointNodes                   _pointNodes;
    R::Nodes*                       _nodeRenderer;
    R::CylinderEdges                _cylinderEdges;
    R::LineEdges                    _lineEdges;
    R::GL_LineEdges                 _glLineEdges;
    R::EdgeRenderer*                _edgeRenderer;

    void RebuildRenderEdges();

    struct Face {
        unsigned idx;
        unsigned sz;
    };
    std::vector<M::Vector3>         _fpos;
    std::vector<leda::node>         _vmap; // maps rendermesh vertex IDs to polymesh vertices
    std::vector<Face>               _faces;
    std::vector<R::Mesh::Vertex>    _vertices;
    std::vector<R::Mesh::Index>     _indices;
    R::Mesh::Desc                   _meshDesc;
    R::meshPtr_t                    _mesh;
    R::tfmeshPtr_t                  _tfmesh;
    bool                            _meshCompiled; // TODO: might be a race cond

    void CacheFPos();
    void RebuildRenderMesh();
    void UpdateRenderMesh();
    void DestroyRenderMesh();

    float       _edgeRadius;
    R::Color    _edgeColor;

    bool                _isSolid; // detectable by raycast
    bool                _isHidden;
    int                 _renderMode;
    unsigned            _renderLayer;
    ShadingMode::Enum   _shadingMode;
    bool                _stylizedHiddenLines;
    R::RenderList       _renderList;

    float               _transparency;
    bool                _isTransparent;

    M::Box              _bbox;      // untransformed bounding box

    void ComputeCenter();
    void ComputeBoundingBox();

    // selection
    std::vector<leda::node> _vertexSelection;

#pragma region EventHandlers
    void Event_EdgeRadiusChanged(const EV::Event& event);
    void Event_EdgeColorChanged(const EV::Event& event);
    void Event_TransparencyChanged(const EV::Event& event);
    void Event_RenderModeChanged(const EV::Event& event);
    void Event_EdgeShadingChanged(const EV::Event& event);
#pragma endregion
public:
    ENT_Geometry();

    DECL_HANDLE_EVENTS(ENT_Geometry);

    struct VertexHit {
        leda::node  vert;
        float       dist;

        VertexHit() { }
        VertexHit(leda::node vert, float dist) : vert(vert), dist(dist) { }
    };

    bool TraceVertices(const M::Ray& ray, float radius, std::vector<VertexHit>& hits);

    void Select() { Entity::Select(); }

    // NOTE: these methods should only be called by ISelection
    void                    Select(const leda::node v);
    void                    ClearVertexSelection();

    std::vector<leda::node> GetVertexSelection() const;

    bool IsSolid() const { return _isSolid; }

    void SetSolid(bool solid) override;

    void SetTransparency(float transparency);

    bool IsMeshCompiled() const;
    void CompileMesh();

    const std::string& GetName() const override { return Entity::GetName(); }

    void SetName(const std::string& name) override;

	void Destroy() override { Entity::Destroy(); }
    void OnDestroy() override;

    UI::OutlinerView* CreateOutlinerView() override;

    leda::nb::RatPolyMesh&          GetRatPolyMesh() override;
    const leda::nb::RatPolyMesh&    GetRatPolyMesh() const;
    void Rebuild();

    void ApplyTransformation() override;

    float       GetEdgeRadius() const;
    R::Color    GetEdgeColor() const;

    void SetEdgeRadius(float edgeRadius);
    void SetEdgeColor(const R::Color& color);

    M::Vector3 GetLocalCenter() const;
    M::Vector3 GetGlobalCenter();

    const M::Box& GetBoundingBox() const;

    M::Vector3 GetPosition() const override;

    void SetPosition(const M::Vector3& position) override;

    void HideOutline() override;

    void Show() override;
    void Hide() override;

    void SetRenderMode(int flags) override;
    void SetRenderLayer(unsigned layer) override;
    void SetShadingMode(ShadingMode::Enum mode) override;

    int GetRenderMode() const { return _renderMode; }

    void SetEditMode(editMode_t::Enum mode);

    void FrameUpdate();
    void BuildRenderList();
    const R::RenderList& GetRenderList() const { return _renderList; }
};

} // namespace W