#include <maxint.h>

#include <Nubuck\polymesh.h>
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

    RemoveDegeneratedEdges(_edges);
    if(_edges.empty()) return;

    _edgeBBoards.clear();
    _edgeBBoards.resize(_edges.size());

    // encodes vertex position
    static const M::Vector2 texCoords[] = {
        M::Vector2(-0.5f,  1.0f),
        M::Vector2(-0.5f,  0.0f),
        M::Vector2( 0.5f,  0.0f),
        M::Vector2( 0.5f,  1.0f)
    };

    unsigned numEdges = _edges.size();
    for(unsigned i = 0; i < numEdges; ++i) {
        const Edge& edge = _edges[i];
        const M::Vector3 axis = edge.p1 - edge.p0;
        for(unsigned j = 0; j < 4; ++j) {
            _edgeBBoards[i].verts[j].position   = edge.p0;

            // since line edges are unlit we can use the normal
            // vector to store the edge axis.
            // note that height = len(axis)
            _edgeBBoards[i].verts[j].normal     = axis;

            _edgeBBoards[i].verts[j].texCoords  = texCoords[j];

            // edgeBBoards[i].verts[j].color    = ColorTo3ub(edge.color);
            // _edgeBBoards[i].verts[j].color      = edge.color;

            // since line edges are opaque we can use the
            // alpha channel to store the edge radius
            _edgeBBoards[i].verts[j].color.a    = edge.radius;
        }

        // set vertex colors
        R::CopyRGB(_edgeBBoards[i].verts[0].color, edge.color1);
        R::CopyRGB(_edgeBBoards[i].verts[1].color, edge.color0);
        R::CopyRGB(_edgeBBoards[i].verts[2].color, edge.color0);
        R::CopyRGB(_edgeBBoards[i].verts[3].color, edge.color1);
    }

    _edgeBBoardIndices.clear();
    for(unsigned i = 0; i < 4 * numEdges; ++i) {
        if(0 < i && 0 == i % 4) _edgeBBoardIndices.push_back(Mesh::RESTART_INDEX);
        _edgeBBoardIndices.push_back(i);
    }
    unsigned numBillboardIndices = M::Max(1u, 5 * numEdges) - 1; // max handles case size = 0
    assert(numBillboardIndices == _edgeBBoardIndices.size());

    // rebuild mesh
    _meshDesc.vertices = &_edgeBBoards[0].verts[0];
    _meshDesc.numVertices = 4 * _edgeBBoards.size();
    _meshDesc.indices = &_edgeBBoardIndices[0];
    _meshDesc.numIndices = _edgeBBoardIndices.size();
    _meshDesc.primType = GL_TRIANGLE_FAN;

    if(_mesh) DestroyMesh();

    if(!_edges.empty()) {
        M::Matrix4 lastTransform = M::Mat4::Identity();
        if(_tfmesh) lastTransform = meshMgr.GetMesh(_tfmesh).GetTransform();

        _mesh = meshMgr.Create(_meshDesc);
        _tfmesh = meshMgr.Create(_mesh);
        meshMgr.GetMesh(_tfmesh).SetTransform(lastTransform);
    }
}

void LineEdges::Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) {
    for(unsigned i = 0; i < _edges.size(); ++i) {
        Edge& edge = _edges[i];
        edge.p0 = fpos[edge.v0->id()];
        edge.p1 = fpos[edge.v1->id()];
        edge.color0 = mesh.color_of(edge.pe);
        edge.color1 = mesh.color_of(mesh.reversal(edge.pe));
    }
    Rebuild(_edges);
}

void LineEdges::SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) {
	SYS::ScopedLock lock(_mtx);
    if(_tfmesh) meshMgr.GetMesh(_tfmesh).SetTransform(transform);
}

void LineEdges::BuildRenderMesh() {
	SYS::ScopedLock lock(_mtx);

    // nothing to do here
}

void LineEdges::DestroyRenderMesh() {
    DestroyMesh();
}

MeshJob LineEdges::GetRenderJob() const {
	SYS::ScopedLock lock(_mtx);

	assert(!_edges.empty());

    R::Material mat = R::Material::White;
    mat.SetUniformBinding("patternColor", R::Color(0.0f, 0.0f, 0.0f, 0.0f));
    mat.SetUniformBinding("patternTex", NULL);

    MeshJob meshJob;
    meshJob.fx = "EdgeLineBillboard";
    meshJob.tfmesh = _tfmesh;
    meshJob.material = mat;
    meshJob.primType = 0;

    return meshJob;
}

} // namespace R
