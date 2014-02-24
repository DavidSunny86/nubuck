#pragma once

#include <vector>
#include <Nubuck\generic\uncopyable.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr_fwd.h>
#include "r_edges.h"

namespace R {

class LineEdges : private GEN::Uncopyable, public EdgeRenderer {
private:
    struct Billboard { Mesh::Vertex verts[4]; };

    mutable SYS::SpinLock _mtx;

    std::vector<Edge>           _edges;
    std::vector<Billboard>      _edgeBBoards;
    std::vector<Mesh::Index>    _edgeBBoardIndices;
	Mesh::Desc                  _meshDesc;
    meshPtr_t               	_mesh;
    tfmeshPtr_t             	_tfmesh;
    bool                    	_needsRebuild;
    bool                    	_isInvalid;

    void DestroyMesh();
public:
    LineEdges() : _mesh(NULL), _tfmesh(NULL), _needsRebuild(false), _isInvalid(false) { }
    ~LineEdges();

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
