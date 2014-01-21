#include <math\matrix3.h>
#include <renderer\renderer_local.h>
#include <renderer\mesh\meshmgr.h>
#include "r_line_edges.h"

namespace R {

void LineEdges::DestroyMesh() {
    if(_mesh) {
        meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
}

LineEdges::~LineEdges() {
    DestroyMesh();
}

void LineEdges::Clear() {
    _edges.clear();
    DestroyMesh();
}

void LineEdges::Rebuild() {
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

    std::vector<Mesh::Index> edgeBBoardIndices;
    for(unsigned i = 0; i < 4 * numEdges; ++i) {
        if(0 < i && 0 == i % 4) edgeBBoardIndices.push_back(Mesh::RESTART_INDEX);
        edgeBBoardIndices.push_back(i);
    }
    unsigned numBillboardIndices = M::Max(1u, 5 * numEdges) - 1; // max handles case size = 0
    assert(numBillboardIndices == edgeBBoardIndices.size());

    assert(NULL == _mesh);

    Mesh::Desc meshDesc;
    meshDesc.vertices = &_edgeBBoards[0].verts[0];
    meshDesc.numVertices = 4 * _edgeBBoards.size();
    meshDesc.indices = &edgeBBoardIndices[0];
    meshDesc.numIndices = edgeBBoardIndices.size();
    meshDesc.primType = GL_TRIANGLE_FAN;

    _mesh = meshMgr.Create(meshDesc);
}

void LineEdges::SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) {
    if(IsEmpty()) return;

    // wEye: eye position in world space
    M::Matrix4 invWorld;
    bool ret = M::TryInvert(modelView, invWorld);
    const M::Vector3 eyePos = M::Transform(invWorld, M::Vector3::Zero);

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

    if(_mesh) meshMgr.GetMesh(_mesh).Invalidate(&_edgeBBoards[0].verts[0]);
}

MeshJob LineEdges::GetRenderJob() const {
    assert(!IsEmpty());

    MeshJob meshJob;
    meshJob.fx = "EdgeLineBillboard";
    meshJob.mesh = _mesh;
    meshJob.material = Material::White;
    meshJob.primType = 0;
    meshJob.transform = M::Mat4::Identity();

    return meshJob;
}

} // namespace R
