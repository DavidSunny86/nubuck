#include <renderer\nodes\r_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
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

    if(0 == _ratPolyMesh.number_of_faces()) return;

    unsigned idxCnt = 0;

    leda::node_array<M::Vector3> fpos(_ratPolyMesh);

    leda::node v;
    forall_nodes(v, _ratPolyMesh) {
        fpos[v] = ToVector(_ratPolyMesh.position_of(v));
    }

    leda::face f;
    forall_faces(f, _ratPolyMesh) {
        if(!_ratPolyMesh.is_visible(f)) continue;

        leda::edge e = _ratPolyMesh.first_face_edge(f);

        leda::edge n = _ratPolyMesh.face_cycle_succ(e);
        const M::Vector3& p0 = fpos[leda::source(e)];
        const M::Vector3& p1 = fpos[leda::target(e)];
        const M::Vector3& p2 = fpos[leda::target(n)];
        const M::Vector3 normal = M::Normalize(M::Cross(p1 - p0, p2 - p0));

        R::Mesh::Vertex vert;
        vert.normal = normal;
        vert.color = R::Color::White;

        leda::edge it = e;
        do {
            vert.position = fpos[leda::source(it)];
            _vertices.push_back(vert);
            _indices.push_back(idxCnt++);
            it = _ratPolyMesh.face_cycle_succ(it);
        } while(e != it);
        _indices.push_back(R::Mesh::RESTART_INDEX);
    } // forall_faces

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
    _renderList.renderJobs.clear();
    _renderList.meshJobs.clear();

    if(RENDER_SOLID == _renderMode && NULL != _mesh) {
        R::MeshJob rjob;
        rjob.fx         = "LitDirectional";
        rjob.material   = R::Material::White;
        rjob.mesh       = _mesh;
        rjob.primType   = 0;
        rjob.transform  = M::Mat4::Identity();
        _renderList.meshJobs.push_back(rjob);
    }

    if(RENDER_SOLID == _renderMode) {
        std::vector<R::Nodes::Node> rnodes;
        leda::node v;
        forall_nodes(v, _ratPolyMesh) {
            R::Nodes::Node rnode;
            rnode.position = ToVector(_ratPolyMesh.position_of(v));
            rnode.color = R::Color(0.3f, 0.3f, 0.3f);
            rnodes.push_back(rnode);
        }
        R::g_nodes.Draw(rnodes);
    }

    std::vector<R::Edge> redges;
    R::Edge re;
    re.radius = 0.02f;
    leda::edge e;
    forall_edges(e, _ratPolyMesh) {
        re.color = R::Color(0.3f, 0.3f, 0.3f);
        re.p0 = ToVector(_ratPolyMesh.position_of(leda::source(e)));
        re.p1 = ToVector(_ratPolyMesh.position_of(leda::target(e)));
        redges.push_back(re);
    }
    R::g_cylinderEdges.Draw(redges);
}

} // namespace W