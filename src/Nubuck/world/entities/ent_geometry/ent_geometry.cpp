#include "ent_geometry.h"

namespace {

M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

} // unnamed namespace

namespace W {

void ENT_Geometry::Update() {
    _vertices.clear();
    _indices.clear();

    unsigned idxCnt = 0;

    std::vector<M::Vector3> positions(_ratPolyMesh.V_MapSize());
    size_t vertex = _ratPolyMesh.V_Begin();
    while(_ratPolyMesh.V_End() != vertex) {
        positions[vertex] = ToVector(_ratPolyMesh.V_GetPosition(vertex));
        vertex = _ratPolyMesh.V_Next(vertex);
    }

    size_t face = _ratPolyMesh.F_Begin();
    while(_ratPolyMesh.F_End() != face) {
        size_t edge = _ratPolyMesh.F_HalfEdge(face);

        size_t next = _ratPolyMesh.H_FaceSucc(edge);
        const M::Vector3& p0 = positions[_ratPolyMesh.H_StartVertex(edge)];
        const M::Vector3& p1 = positions[_ratPolyMesh.H_StartVertex(next)];
        const M::Vector3& p2 = positions[_ratPolyMesh.H_EndVertex(next)];
        const M::Vector3 normal = M::Normalize(M::Cross(p1 - p0, p2 - p0));

        size_t it = edge;
        R::Mesh::Vertex vert;
        vert.color = R::Color::White;
        vert.normal = normal;
        do {
            vert.position = positions[_ratPolyMesh.H_StartVertex(it)];
            _vertices.push_back(vert);
            _indices.push_back(idxCnt++);
            it = _ratPolyMesh.H_FaceSucc(it);
        } while(edge != it);
        _indices.push_back(R::Mesh::RESTART_INDEX);
        face = _ratPolyMesh.F_Next(face);
    }

    _meshDesc.vertices = &_vertices[0];
    _meshDesc.numVertices = _vertices.size();
    _meshDesc.indices = &_indices[0];
    _meshDesc.numIndices = _indices.size();
    _meshDesc.primType = GL_TRIANGLE_FAN;

    _meshCompiled = false;
}

void ENT_Geometry::CompileMesh() {
    if(_meshCompiled) return;

    if(NULL != _mesh) R::meshMgr.Destroy(_mesh);
    _mesh = R::meshMgr.Create(_meshDesc);

    _meshCompiled = true;
}

void ENT_Geometry::BuildRenderList() {
    _renderList.meshJobs.clear();

    if(NULL == _mesh) return;

    R::MeshJob rjob;
    rjob.fx         = "LitDirectional";
    rjob.material   = R::Material::White;
    rjob.mesh       = _mesh;
    rjob.primType   = 0;
    rjob.transform  = M::Mat4::Identity();
    _renderList.meshJobs.push_back(rjob);
}

} // namespace W