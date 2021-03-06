#include <maxint.h>

#include <Nubuck\polymesh.h>
#include <renderer\mesh\meshmgr.h>
#include "r_gl_line_edges.h"

namespace R {

void GL_LineEdges::DestroyMesh() {
    if(_mesh) {
        meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
    if(_tfmesh) {
        meshMgr.Destroy(_tfmesh);
        _tfmesh = NULL;
    }
}

GL_LineEdges::GL_LineEdges()
    : _mesh(NULL)
    , _tfmesh(NULL)
{ }

void GL_LineEdges::Rebuild(const std::vector<Edge>& edges) {
    _edges = edges;

    RemoveDegeneratedEdges(_edges);
    if(_edges.empty()) return;

    const unsigned numEdges = _edges.size();
    const unsigned numVerts = 2 * numEdges;

    _meshVertices.resize(numVerts);
    for(unsigned i = 0; i < numEdges; ++i) {
        const Edge& edge = _edges[i];
        const unsigned v0 = 2 * i;
        const unsigned v1 = v0 + 1;
        _meshVertices[v0].position  = edge.p0;
        _meshVertices[v1].position  = edge.p1;
        _meshVertices[v0].color     = edge.color0;
        _meshVertices[v1].color     = edge.color1;

        // since line edges are opaque we can use the
        // alpha channel to store the edge radius
        _meshVertices[v0].color.a   = edge.radius;
        _meshVertices[v1].color.a   = edge.radius;
    }

    _meshIndices.resize(numVerts);
    for(unsigned i = 0; i < numVerts; ++i)
        _meshIndices[i] = i;

    // rebuild mesh
    _meshDesc.vertices      = &_meshVertices[0];
    _meshDesc.numVertices   = numVerts;
    _meshDesc.indices       = &_meshIndices[0];
    _meshDesc.numIndices    = numVerts;
    _meshDesc.primType      = GL_LINES;

    if(_mesh) DestroyMesh();

    if(!_edges.empty()) {
        M::Matrix4 lastTransform = M::Mat4::Identity();
        if(_tfmesh) lastTransform = meshMgr.GetMesh(_tfmesh).GetTransform();

        _mesh = meshMgr.Create(_meshDesc);
        _tfmesh = meshMgr.Create(_mesh);
        meshMgr.GetMesh(_tfmesh).SetTransform(lastTransform);
    }
}

void GL_LineEdges::Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) {
    for(unsigned i = 0; i < _edges.size(); ++i) {
        Edge& edge = _edges[i];
        edge.p0 = fpos[edge.v0->id()];
        edge.p1 = fpos[edge.v1->id()];
        edge.color0 = mesh.color_of(edge.pe);
        edge.color1 = mesh.color_of(mesh.reversal(edge.pe));
    }
    Rebuild(_edges);
}

void GL_LineEdges::SetTransform(const M::Matrix4& transform, const M::Matrix4& modelView) {
    if(_tfmesh) meshMgr.GetMesh(_tfmesh).SetTransform(transform);
}

void GL_LineEdges::BuildRenderMesh() {
    // nothing to do here
}

void GL_LineEdges::DestroyRenderMesh() {
    DestroyMesh();
}

MeshJob GL_LineEdges::GetRenderJob() const {
    assert(!_edges.empty());

    R::Material mat = R::Material::White;
    mat.SetUniformBinding("patternColor", R::Color(0.0f, 0.0f, 0.0f, 0.0f));
    mat.SetUniformBinding("patternTex", NULL);

    MeshJob meshJob;
    meshJob.fx          = "UnlitThickLines";
    meshJob.tfmesh      = _tfmesh;
    meshJob.material    = mat;
    meshJob.primType    = 0;

    return meshJob;
}

} // namespace R