#include <Nubuck\math\matrix3.h>
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

void Nodes::Rebuild(const std::vector<Node>& nodes) {
	SYS::ScopedLock lock(_mtx);

    _nodes = nodes;

    unsigned numBillboards = _nodes.size();
    unsigned numBillboardIndices = 5 * numBillboards - 1;

    _billboards.clear();
    _billboards.resize(numBillboards);

    static const M::Vector2 bbTexCoords[4] = {
        M::Vector2(-1.0f, -1.0f),
        M::Vector2( 1.0f, -1.0f),
        M::Vector2( 1.0f,  1.0f),
        M::Vector2(-1.0f,  1.0f)
    };

    for(unsigned i = 0; i < numBillboards; ++i) {
        for(unsigned k = 0; k < 4; ++k) {
            _billboards[i].verts[k].color = _nodes[i].color;
            _billboards[i].verts[k].texCoords = bbTexCoords[k];
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
    for(unsigned i = 0; i < _nodes.size(); ++i)
        _nodes[i].position = fpos[_nodes[i].pvert->id()];
    _isInvalid = true;
}

void Nodes::Transform(const M::Matrix4& modelView) {
	SYS::ScopedLock lock(_mtx);

	if(_nodes.empty()) return;

	float nodeSize = cvar_r_nodeSize;
    const M::Vector3 bbPositions[4] = {
        M::Vector3(-nodeSize, -nodeSize, 0.0f),
        M::Vector3( nodeSize, -nodeSize, 0.0f),
        M::Vector3( nodeSize,  nodeSize, 0.0f),
        M::Vector3(-nodeSize,  nodeSize, 0.0f)
    };

    for(unsigned i = 0; i < _billboards.size(); ++i) {
        for(unsigned k = 0; k < 4; ++k) {
            _billboards[i].verts[k].position = M::Transform(modelView, _nodes[i].position) + bbPositions[k];
        }
    }

    _isInvalid = true;
}

void Nodes::BuildRenderMesh() {
	SYS::ScopedLock lock(_mtx);

    unsigned numBillboards = _nodes.size();
    unsigned numBillboardIndices = 5 * numBillboards - 1;

    if(_isInvalid && _mesh) {
		meshMgr.GetMesh(_mesh).Invalidate(&_billboards[0].verts[0]);
        _isInvalid = false;
	}

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
			meshMgr.GetMesh(_tfmesh).SetTransform(M::Mat4::Identity());
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