#include <Nubuck\math\matrix3.h>
#include <renderer\renderer_local.h>
#include <renderer\mesh\meshmgr.h>
#include "r_line_edges.h"

namespace R {

void LineEdges::DestroyMesh() {
    if(_mesh) {
        meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
    if(_tfmesh) {
        meshMgr.Destroy(_tfmesh);
        _tfmesh = NULL;
    }
}

LineEdges::~LineEdges() {
}

void LineEdges::Rebuild(const std::vector<Edge>& edges) {
	SYS::ScopedLock lock(_mtx);

    _edges = edges;
    _needsRebuild = true;

    RemoveDegeneratedEdges(_edges);
    if(_edges.empty()) return;

    _edgeBBoards.clear();
    _edgeBBoards.resize(_edges.size());

    unsigned numEdges = _edges.size();
    for(unsigned i = 0; i < numEdges; ++i) {
        const Edge& edge = _edges[i];
        for(unsigned j = 0; j < 4; ++j) {
            // edgeBBoards[i].verts[j].color = ColorTo3ub(edge.color);
            _edgeBBoards[i].verts[j].color = edge.color;
        }
    }

    _edgeBBoardIndices.clear();
    for(unsigned i = 0; i < 4 * numEdges; ++i) {
        if(0 < i && 0 == i % 4) _edgeBBoardIndices.push_back(Mesh::RESTART_INDEX);
        _edgeBBoardIndices.push_back(i);
    }
    unsigned numBillboardIndices = M::Max(1u, 5 * numEdges) - 1; // max handles case size = 0
    assert(numBillboardIndices == _edgeBBoardIndices.size());

    _meshDesc.vertices = &_edgeBBoards[0].verts[0];
    _meshDesc.numVertices = 4 * _edgeBBoards.size();
    _meshDesc.indices = &_edgeBBoardIndices[0];
    _meshDesc.numIndices = _edgeBBoardIndices.size();
    _meshDesc.primType = GL_TRIANGLE_FAN;
}

void LineEdges::SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) {
	SYS::ScopedLock lock(_mtx);

    if(_edges.empty()) return;

    static const M::Vector3 vertPos[] = {
        M::Vector3(-0.5f,  1.0f, 0.0f),
        M::Vector3(-0.5f,  0.0f, 0.0f),
        M::Vector3( 0.5f,  0.0f, 0.0f),
        M::Vector3( 0.5f,  1.0f, 0.0f)
    };

    unsigned numEdges = _edges.size();
    for(unsigned i = 0; i < numEdges; ++i) {
        const Edge& edge = _edges[i];

        const M::Vector3 p0 = M::Transform(modelView, M::Transform(transform, edge.p0));
        const M::Vector3 p1 = M::Transform(modelView, M::Transform(transform, edge.p1));

        const M::Vector3 wAxisY = M::Normalize(p1 - p0);
        const M::Vector3 wAxisX = M::Normalize(M::Cross(wAxisY, -p0));
        const M::Vector3 wAxisZ = M::Normalize(M::Cross(wAxisX, wAxisY));
        const M::Matrix3 rotate = M::Mat3::FromColumns(wAxisX, wAxisY, wAxisZ); // local to world space

        const float height = M::Distance(p0, p1);
        const M::Matrix3 scale = M::Mat3::Scale(edge.radius, height, 1.0f);

        const M::Matrix3 M = rotate * scale;

        for(unsigned j = 0; j < 4; ++j) {
            _edgeBBoards[i].verts[j].position = p0 + M::Transform(M, vertPos[j]);
        }
    }

    _isInvalid = true;
}

void LineEdges::BuildRenderMesh() {
	SYS::ScopedLock lock(_mtx);

    if(_needsRebuild) {
        if(_mesh) DestroyMesh();

		if(!_edges.empty()) {
            _mesh = meshMgr.Create(_meshDesc);
            _tfmesh = meshMgr.Create(_mesh);
            meshMgr.GetMesh(_tfmesh).SetTransform(M::Mat4::Identity());
		}

        _needsRebuild = false;
	}

    if(_isInvalid && _mesh) {
        meshMgr.GetMesh(_mesh).Invalidate(&_edgeBBoards[0].verts[0]);
        _isInvalid = false;
	}
}

void LineEdges::DestroyRenderMesh() {
    DestroyMesh();
}

MeshJob LineEdges::GetRenderJob() const {
	SYS::ScopedLock lock(_mtx);

	assert(!_edges.empty());

    MeshJob meshJob;
    meshJob.fx = "EdgeLineBillboard";
    meshJob.tfmesh = _tfmesh;
    meshJob.material = Material::White;
    meshJob.primType = 0;

    return meshJob;
}

} // namespace R
