#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <math\box.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include <renderer\nodes\r_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include <UI\outliner\outliner.h>
#include <world\entity.h>

namespace W {

class ENT_Geometry : public QObject, public IGeometry, public Entity {
    Q_OBJECT
private:
    leda::nb::RatPolyMesh _ratPolyMesh;

    R::Nodes                        _nodes;
    R::CylinderEdges                _cylinderEdges;
    R::LineEdges                    _lineEdges;
    R::EdgeRenderer*                _edgeRenderer;

    void RebuildEdges();

    std::vector<R::Mesh::Vertex>    _vertices;
    std::vector<R::Mesh::Vertex>    _tfverts;
    std::vector<R::Mesh::Index>     _indices;
    R::Mesh::Desc                   _meshDesc;
    R::meshPtr_t                    _mesh;
    R::tfmeshPtr_t                  _tfmesh;
    bool                            _meshCompiled; // TODO: might be a race cond

    float       _edgeRadius;
    R::Color    _edgeColor;

    bool                _isHidden;
    int                 _renderMode;
    unsigned            _renderLayer;
    ShadingMode::Enum   _shadingMode;
    R::RenderList       _renderList;

    M::Box              _bbox;      // untransformed bounding box

    struct {
		UI::Outliner::itemHandle_t  item;

        QDoubleSpinBox*             sbEdgeRadius;
        UI::ColorButton*    		btnEdgeColor;
        QSlider*            		sldHullAlpha;
	} _outln;

    void InitOutline();

    void TransformVertices();
    void ComputeCenter();
    void ComputeBoundingBox();
private slots:
    void OnEdgeRadiusChanged(double value);
    void OnEdgeColorChanged(float r, float g, float b);
public:
    ENT_Geometry();

    bool IsMeshCompiled() const { return _meshCompiled; }
    void CompileMesh();

    void Destroy() override { 
        if(_outln.item) {
			UI::Outliner::Instance()->RemoveItem(_outln.item);
			_outln.item = NULL;
		}
		Entity::Destroy(); 
	}

    leda::nb::RatPolyMesh& GetRatPolyMesh() override { return _ratPolyMesh; }
    void Update() override;

    void ApplyTransformation();

    void SetEdgeRadius(float edgeRadius) {
        _edgeRadius = edgeRadius;
		_outln.sbEdgeRadius->blockSignals(true);
		_outln.sbEdgeRadius->setValue(_edgeRadius);
		_outln.sbEdgeRadius->blockSignals(false);
        RebuildEdges();
	}

    void SetEdgeColor(const R::Color& color) {
        _edgeColor = color;
		_outln.btnEdgeColor->blockSignals(true);
		_outln.btnEdgeColor->SetColor(_edgeColor.r, _edgeColor.g, _edgeColor.b);
		_outln.btnEdgeColor->blockSignals(false);
        RebuildEdges();
	}

    const M::Vector3& GetLocalCenter() const { return _bbox.min + 0.5f * (_bbox.max - _bbox.min); }
    M::Vector3 GetGlobalCenter() { return Transform(GetLocalCenter()); }

    const M::Box& GetBoundingBox() const { return _bbox; }

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