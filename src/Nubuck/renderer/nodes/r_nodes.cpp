#include <Nubuck\math\matrix3.h>
#include <Nubuck\polymesh.h>
#include <renderer\mesh\meshmgr.h>
#include "r_nodes.h"

namespace R {

void Nodes::DestroyMesh() {
    if(_mesh) {
        meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
    if(_tfmesh) {
        meshMgr.Destroy(_tfmesh);
        _tfmesh = NULL;
    }
}

Nodes::~Nodes() {
    // DestroyMesh();
}

void Nodes::Rebuild(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) {
	SYS::ScopedLock lock(_mtx);

    _nodes.clear();
    _inMap.clear();
    _inMap.resize(mesh.max_node_index() + 1);

    Node        rv;
    leda::node  pv;
    forall_nodes(pv, mesh) {
        rv.pvert    = pv;
        rv.position = fpos[pv->id()];
        rv.color    = mesh.color_of(pv);

        _inMap[pv->id()] = _nodes.size();
        _nodes.push_back(rv);
    }

    unsigned numBillboards = _nodes.size();
    unsigned numBillboardIndices = 5 * numBillboards - 1;

    _billboards.clear();
    _billboards.resize(numBillboards);

    // bbNormals.xy encode texcoords
	float nodeSize = cvar_r_nodeSize;
    static const M::Vector3 bbNormals[4] = {
        M::Vector3(-1.0f, -1.0f, nodeSize),
        M::Vector3( 1.0f, -1.0f, nodeSize),
        M::Vector3( 1.0f,  1.0f, nodeSize),
        M::Vector3(-1.0f,  1.0f, nodeSize)
    };

    for(unsigned i = 0; i < numBillboards; ++i) {
        for(unsigned k = 0; k < 4; ++k) {
            _billboards[i].verts[k].position = _nodes[i].position;
            _billboards[i].verts[k].color = _nodes[i].color;
            _billboards[i].verts[k].normal = bbNormals[k];
        }
    }

    _billboardIndices.clear();
    _billboardIndices.reserve(numBillboardIndices);
    for(unsigned i = 0; i < 4 * numBillboards; ++i) {
        if(0 < i && 0 == i % 4) _billboardIndices.push_back(Mesh::RESTART_INDEX);
        _billboardIndices.push_back(i);
    }
    assert(numBillboardIndices == _billboardIndices.size());

    _needsRebuild = true;
}

void Nodes::Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) {
    typedef leda::nb::RatPolyMesh::State state_t;
    for(unsigned i = 0; i < _nodes.size(); ++i) {
        leda::node pv = _nodes[i].pvert;
        if(state_t::GEOMETRY_CHANGED == mesh.state_of(pv)) {
            _nodes[i].position = fpos[_nodes[i].pvert->id()];
            for(unsigned k = 0; k < 4; ++k)
                _billboards[i].verts[k].position = _nodes[i].position;

            const unsigned vertSz = sizeof(Mesh::Vertex);
            const unsigned off = vertSz * (&_billboards[i].verts[0] - &_billboards[0].verts[0]);
            const unsigned size = 4 * vertSz;
            if(_mesh) meshMgr.GetMesh(_mesh).Invalidate(&_billboards[0].verts[0], off, size);
        }
    }
}

void Nodes::SetColor(leda::node pv, const Color& color) {
    const unsigned ridx = _inMap[pv->id()];

    _nodes[ridx].color = color;
    for(unsigned k = 0; k < 4; ++k)
        _billboards[ridx].verts[k].color = _nodes[ridx].color;

    if(_mesh) {
        const unsigned vertSz = sizeof(Mesh::Vertex);
        const unsigned off = vertSz * (&_billboards[ridx].verts[0] - &_billboards[0].verts[0]);
        const unsigned size = 4 * vertSz;

        meshMgr.GetMesh(_mesh).Invalidate(&_billboards[0].verts[0], off, size);
    }
}

void Nodes::Transform(const M::Matrix4& objToWorld) {
	SYS::ScopedLock lock(_mtx);
    if(_tfmesh) R::meshMgr.GetMesh(_tfmesh).SetTransform(objToWorld);
}

void Nodes::BuildRenderMesh() {
	SYS::ScopedLock lock(_mtx);

    unsigned numBillboards = _nodes.size();
    unsigned numBillboardIndices = 5 * numBillboards - 1;

    if(_isInvalid && _mesh) {
		meshMgr.GetMesh(_mesh).Invalidate(&_billboards[0].verts[0]);
        _isInvalid = false;
	}

    M::Matrix4 lastTransform = M::Mat4::Identity();
    if(_tfmesh) lastTransform = meshMgr.GetMesh(_tfmesh).GetTransform();

    if(_needsRebuild && !_nodes.empty()) {
        if(_mesh) DestroyMesh();

		if(!_nodes.empty()) {
			Mesh::Desc meshDesc;
			meshDesc.vertices = &_billboards[0].verts[0];
			meshDesc.numVertices = 4 * numBillboards;
			meshDesc.indices = &_billboardIndices[0];
			meshDesc.numIndices = numBillboardIndices;
			meshDesc.primType = GL_TRIANGLE_FAN;

			_mesh = meshMgr.Create(meshDesc);
			_tfmesh = meshMgr.Create(_mesh);
			meshMgr.GetMesh(_tfmesh).SetTransform(lastTransform);
		}

        _needsRebuild = false;
    }
}

void Nodes::DestroyRenderMesh() {
    DestroyMesh();
}

MeshJob Nodes::GetRenderJob() const {
	SYS::ScopedLock lock(_mtx);

	assert(!_nodes.empty());

    R::MeshJob meshJob;
    meshJob.fx = "NodeBillboard";
    meshJob.tfmesh = _tfmesh;
    meshJob.material = Material::White;
    meshJob.primType = 0;
    return meshJob;
}

} // namespace R