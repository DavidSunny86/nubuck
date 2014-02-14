#pragma once

#include <generic\uncopyable.h>
#include <renderer\renderer.h>
#include <renderer\mesh\meshmgr_fwd.h>
#include "r_edges.h"

namespace R {

class CylinderEdges : private GEN::Uncopyable, public EdgeRenderer {
private:
    struct FatEdge : Edge {
        M::Matrix4 Rt; // object to intertia space for untransformed edges
        explicit FatEdge(const Edge& e) : Edge(e) { }
    };

    mutable SYS::SpinLock _mtx;

    std::vector<FatEdge>        _edges;
    std::vector<Mesh::Vertex>   _edgeBBoxVertices;
    std::vector<Mesh::Index>    _edgeBBoxIndices;
    Mesh::Desc                  _meshDesc; 
    meshPtr_t                   _mesh;
    tfmeshPtr_t                 _tfmesh;
    bool                        _needsRebuild;
    bool                        _isInvalid;

    void DestroyMesh();
    void RebuildVertices(const M::Matrix4& transform);
public:
    CylinderEdges() : _mesh(NULL), _tfmesh(NULL), _needsRebuild(false), _isInvalid(false) { }
    ~CylinderEdges();

    bool IsEmpty() const override { 
		SYS::ScopedLock lock(_mtx);
		return _edges.empty(); 
	}

    void Rebuild(const std::vector<Edge>& edges) override;

    void SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) override;

    void BuildRenderMesh() override;
    void DestroyRenderMesh() override;

    MeshJob GetRenderJob() const override;
};

} // namespace R