#include <world\world.h>
#include "ent_geometry.h"

namespace {

M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

} // unnamed namespace

namespace W {

void ENT_Geometry::TransformVertices() {
    _tfverts = _vertices;
    for(unsigned i = 0; i < _vertices.size(); ++i) {
        _tfverts[i].position = Transform(_vertices[i].position);
    }
}

void ENT_Geometry::ComputeCenter() {
    leda::node_array<M::Vector3> fpos(_ratPolyMesh);

    _center = M::Vector3::Zero;
    leda::node v;
    forall_nodes(v, _ratPolyMesh) {
        _center += ToVector(_ratPolyMesh.position_of(v));
    }
    _center *= 1.0f / _ratPolyMesh.number_of_nodes();
}

ENT_Geometry::ENT_Geometry() : 
_edgeRenderer(NULL),
_mesh(NULL), 
_tfmesh(NULL), 
_meshCompiled(true), 
_isHidden(false),
_renderMode(0), 
_renderLayer(0),
_shadingMode(ShadingMode::NICE)
{ 
    // _edgeRenderer = &_cylinderEdges;
    _edgeRenderer = &_lineEdges;
}

void ENT_Geometry::Update() {
    leda::node v;
    leda::edge e;

    _nodes.Clear();
    forall_nodes(v, _ratPolyMesh) {
        R::Nodes::Node rnode;
        rnode.position = ToVector(_ratPolyMesh.position_of(v));
        rnode.color = R::Color(0.3f, 0.3f, 0.3f);
        _nodes.Push(rnode);
    }
    _nodes.Rebuild();

    _edgeRenderer->Clear();
    R::EdgeRenderer::Edge re;
    re.radius = 0.02f;
    forall_edges(e, _ratPolyMesh) {
        re.color = R::Color(0.3f, 0.3f, 0.3f);
        re.p0 = ToVector(_ratPolyMesh.position_of(leda::source(e)));
        re.p1 = ToVector(_ratPolyMesh.position_of(leda::target(e)));
        _edgeRenderer->Push(re);
    }
    _edgeRenderer->Rebuild();

    ComputeCenter();

    _vertices.clear();
    _indices.clear();

    if(0 == _ratPolyMesh.number_of_faces()) return;

    unsigned idxCnt = 0;

    leda::node_array<M::Vector3> fpos(_ratPolyMesh);

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
        vert.color = _ratPolyMesh.color_of(f);

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

    if(NULL != _tfmesh) R::meshMgr.Destroy(_tfmesh);
    _tfmesh = R::meshMgr.Create(_mesh);
    R::meshMgr.GetMesh(_tfmesh).SetTransform(M::Mat4::Identity());

    _meshCompiled = true;
}

void ENT_Geometry::Rotate(float ang, float x, float y, float z) {
    GetTransform().rotation = M::RotationOf(M::Mat4::RotateAxis(M::Normalize(M::Vector3(x, y, z)), ang)) * GetTransform().rotation;
}

void ENT_Geometry::FrameUpdate() {
    M::Matrix4 tf = M::Mat4::FromRigidTransform(GetTransform().rotation, GetTransform().position);
    _nodes.Transform(world.GetCameraMatrix() * tf);
    _edgeRenderer->SetTransform(tf, world.GetCameraMatrix());
}

void ENT_Geometry::BuildRenderList() {
    _renderList.meshJobs.clear();

    if(_isHidden) return;

    if(RenderMode::FACES & _renderMode && NULL != _mesh) {
        TransformVertices();
        R::meshMgr.GetMesh(_mesh).Invalidate(&_tfverts[0]);

        R::MeshJob rjob;
        rjob.layer      = _renderLayer;
        rjob.fx         = "LitDirectional";
        rjob.material   = R::Material::White;
        rjob.tfmesh     = _tfmesh;
        rjob.primType   = 0;
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::NODES & _renderMode && ShadingMode::NICE == _shadingMode && !_nodes.IsEmpty()) {
        R::MeshJob rjob = _nodes.GetRenderJob();
        rjob.layer = _renderLayer;
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::EDGES & _renderMode && ShadingMode::NICE == _shadingMode && !_edgeRenderer->IsEmpty()) {
        R::MeshJob rjob = _edgeRenderer->GetRenderJob();
        rjob.layer = _renderLayer;
        _renderList.meshJobs.push_back(rjob);
    }
}

} // namespace W