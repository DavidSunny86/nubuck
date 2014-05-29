#pragma once

#include <vector>

#include <Nubuck\renderer\color\color.h>

#include <Nubuck\generic\uncopyable.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr_fwd.h>

namespace R {

class BillboardNodes : private GEN::Uncopyable {
public:
    struct Node {
        leda::node  pvert; // corresponding vertex of polymesh
        M::Vector3  position;
        R::Color    color;
    };
private:
    struct Billboard { Mesh::Vertex verts[4]; };

	mutable SYS::SpinLock _mtx;

    std::vector<unsigned>           _inMap; // maps polymesh vertex IDs to corresponding indices in _nodes
    std::vector<Node>               _nodes;
    std::vector<Billboard>          _billboards;
    std::vector<Mesh::Index>        _billboardIndices;
    meshPtr_t                       _mesh;
    tfmeshPtr_t                     _tfmesh;
    bool                            _needsRebuild;
    bool                            _isInvalid;

    void DestroyMesh();
public:
    BillboardNodes() : _mesh(NULL), _tfmesh(NULL), _needsRebuild(false), _isInvalid(false) { }
    ~BillboardNodes();

    bool IsEmpty() const { 
		SYS::ScopedLock lock(_mtx);
		return _nodes.empty(); 
	}

	void Rebuild(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos);
    void Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos);

    void SetColor(leda::node pv, const Color& color);

    void Transform(const M::Matrix4& objToWorld);

    void BuildRenderMesh();
    void DestroyRenderMesh();

    R::MeshJob GetRenderJob() const;    
};

} // namespace R