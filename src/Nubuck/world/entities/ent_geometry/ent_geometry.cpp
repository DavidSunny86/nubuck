#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world.h>
#include "ent_geometry_outln.h"
#include "ent_geometry.h"

namespace {

M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

} // unnamed namespace

namespace W {

void ENT_Geometry::CacheFPos() {
    leda::node v;
    _fpos.resize(_ratPolyMesh.max_node_index() + 1);
    forall_nodes(v, _ratPolyMesh) _fpos[v->id()] = ToVector(_ratPolyMesh.position_of(v));
}

void ENT_Geometry::RebuildRenderMesh() {
    _vertices.clear();
    _indices.clear();

    if(0 == _ratPolyMesh.number_of_faces()) return;

    unsigned idxCnt = 0;

    leda::face f;
    forall_faces(f, _ratPolyMesh) {
        if(!_ratPolyMesh.is_visible(f)) continue;

        leda::edge e = _ratPolyMesh.first_face_edge(f);

        leda::edge n = _ratPolyMesh.face_cycle_succ(e);
        const M::Vector3& p0 = _fpos[leda::source(e)->id()];
        const M::Vector3& p1 = _fpos[leda::target(e)->id()];
        const M::Vector3& p2 = _fpos[leda::target(n)->id()];
        const M::Vector3 normal = M::Normalize(M::Cross(p1 - p0, p2 - p0));

        R::Mesh::Vertex vert;
        vert.normal = normal;
        vert.color = _ratPolyMesh.color_of(f);

        leda::edge it = e;
        do {
            vert.position = _fpos[leda::source(it)->id()];
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

void ENT_Geometry::ComputeBoundingBox() {
    _bbox.min = _bbox.max = _fpos[_ratPolyMesh.first_node()->id()];
    leda::node v;
    forall_nodes(v, _ratPolyMesh) {
        M::Vector3 p = _fpos[v->id()];
        _bbox.min.x = M::Min(_bbox.min.x, p.x);
        _bbox.min.y = M::Min(_bbox.min.y, p.y);
        _bbox.min.z = M::Min(_bbox.min.z, p.z);
        _bbox.max.x = M::Max(_bbox.max.x, p.x);
        _bbox.max.y = M::Max(_bbox.max.y, p.y);
        _bbox.max.z = M::Max(_bbox.max.z, p.z);
    }
}

void ENT_Geometry::Event_EdgeRadiusChanged(const EV::Event& event) {
	SYS::ScopedLock lock(_mtx);
	const EV::Params_ENT_Geometry_EdgeRadiusChanged& args = EV::def_ENT_Geometry_EdgeRadiusChanged.GetArgs(event);
	_edgeRadius = args.edgeRadius;
    RebuildRenderEdges();
}

void ENT_Geometry::Event_EdgeColorChanged(const EV::Event& event) {
	SYS::ScopedLock lock(_mtx);
	const EV::Params_ENT_Geometry_EdgeColorChanged& args = EV::def_ENT_Geometry_EdgeColorChanged.GetArgs(event);
	_edgeColor = args.edgeColor;
    RebuildRenderEdges();
}

ENT_Geometry::ENT_Geometry() 
    : _outlinerItem(NULL)
    , _edgeRenderer(NULL)
    , _mesh(NULL)
    , _tfmesh(NULL)
    , _meshCompiled(true)
    , _isHidden(false)
    , _renderMode(0)
    , _renderLayer(0)
    , _shadingMode(ShadingMode::NICE)
{ 
    _edgeRenderer = &_cylinderEdges;
    // _edgeRenderer = &_lineEdges;

    _edgeRadius = 0.02f;
    _edgeColor = R::Color(0.3f, 0.3f, 0.3f);

    _outlinerItem = UI::Outliner::Instance()->AddItem("Polyhedron 0", this);

	AddEventHandler(EV::def_ENT_Geometry_EdgeRadiusChanged, this, &ENT_Geometry::Event_EdgeRadiusChanged);
	AddEventHandler(EV::def_ENT_Geometry_EdgeColorChanged, this, &ENT_Geometry::Event_EdgeColorChanged);
}

UI::OutlinerView* ENT_Geometry::CreateOutlinerView() {
    return new ENT_GeometryOutln(*this);
}

bool ENT_Geometry::IsMeshCompiled() const { return _meshCompiled; }

void ENT_Geometry::RebuildRenderNodes() {
    leda::node v;

	std::vector<R::Nodes::Node> nodes;
    forall_nodes(v, _ratPolyMesh) {
        R::Nodes::Node rnode;
        rnode.position = _fpos[v->id()];
        rnode.color = _ratPolyMesh.color_of(v);
		nodes.push_back(rnode);
    }
    _nodes.Rebuild(nodes);
}

void ENT_Geometry::RebuildRenderEdges() {
    // TODO: don't draw reversal edges!
	std::vector<R::EdgeRenderer::Edge> edges;
    R::EdgeRenderer::Edge re;
    leda::edge e;
    forall_edges(e, _ratPolyMesh) {
		re.radius = _ratPolyMesh.radius_of(e);
		re.color = _ratPolyMesh.color_of(e);
        re.p0 = ToVector(_ratPolyMesh.position_of(leda::source(e)));
        re.p1 = ToVector(_ratPolyMesh.position_of(leda::target(e)));
		edges.push_back(re);
    }
    _edgeRenderer->Rebuild(edges);
}

void ENT_Geometry::Update() {
	SYS::ScopedLock lock(_mtx);

    CacheFPos();
    RebuildRenderNodes();
    RebuildRenderEdges();
    RebuildRenderMesh();
    ComputeBoundingBox();
}

static leda::d3_rat_point ToRatPoint(const M::Vector3& v) {
    return leda::d3_rat_point(leda::d3_point(v.x, v.y, v.z));
}

void ENT_Geometry::ApplyTransformation() {
	SYS::ScopedLock lock(_mtx);

    leda::node n;
    forall_nodes(n, _ratPolyMesh) {
        M::Vector3 pos = Transform(ToVector(_ratPolyMesh.position_of(n)));
        _ratPolyMesh.set_position(n, ToRatPoint(pos));
    }

    EntTransform transform;
    transform.position = M::Vector3::Zero;
    transform.scale = M::Vector3(1.0f, 1.0f, 1.0f);
    transform.rotation = M::Mat3::Identity();
    SetTransform(transform);
}

float ENT_Geometry::GetEdgeRadius() const {
	SYS::ScopedLock lock(_mtx);
    return _edgeRadius;
}

R::Color ENT_Geometry::GetEdgeColor() const {
	SYS::ScopedLock lock(_mtx);
    return _edgeColor;
}

void ENT_Geometry::SetEdgeRadius(float edgeRadius) {
	SYS::ScopedLock lock(_mtx);
    _edgeRadius = edgeRadius;
    RebuildRenderEdges();

    EV::Params_ENT_Geometry_EdgeRadiusChanged args = { _edgeRadius };
    EV::Event event = EV::def_ENT_Geometry_EdgeRadiusChanged.Create(args);
    UI::Outliner::Instance()->SendToView(_outlinerItem, event);
}

void ENT_Geometry::SetEdgeColor(const R::Color& color) {
	SYS::ScopedLock lock(_mtx);
    _edgeColor = color;
    RebuildRenderEdges();

    EV::Params_ENT_Geometry_EdgeColorChanged args = { _edgeColor };
    EV::Event event = EV::def_ENT_Geometry_EdgeColorChanged.Create(args);
    UI::Outliner::Instance()->SendToView(_outlinerItem, event);
}

const M::Vector3& ENT_Geometry::GetLocalCenter() const { 
	SYS::ScopedLock lock(_mtx);
    return _bbox.min + 0.5f * (_bbox.max - _bbox.min); 
}

M::Vector3 ENT_Geometry::GetGlobalCenter() { 
	SYS::ScopedLock lock(_mtx);
    return Transform(GetLocalCenter()); 
}

const M::Box& ENT_Geometry::GetBoundingBox() const { 
	SYS::ScopedLock lock(_mtx);
	return _bbox; 
}

void ENT_Geometry::GetPosition(float& x, float& y, float& z) const {
	SYS::ScopedLock lock(_mtx);
    x = GetTransform().position.x;
    y = GetTransform().position.y;
    z = GetTransform().position.z;
}

void ENT_Geometry::SetPosition(float x, float y, float z) { 
	SYS::ScopedLock lock(_mtx);
	GetTransform().position = M::Vector3(x, y, z);
}

void ENT_Geometry::CompileMesh() {
	SYS::ScopedLock lock(_mtx);

    if(_meshCompiled) return;

    if(NULL != _mesh) R::meshMgr.Destroy(_mesh);
    _mesh = R::meshMgr.Create(_meshDesc);

    if(NULL != _tfmesh) R::meshMgr.Destroy(_tfmesh);
    _tfmesh = R::meshMgr.Create(_mesh);
    R::meshMgr.GetMesh(_tfmesh).SetTransform(GetTransformationMatrix());

    _meshCompiled = true;
}

void ENT_Geometry::OnDestroy() {
	SYS::ScopedLock lock(_mtx);
	_nodes.DestroyRenderMesh();
	_edgeRenderer->DestroyRenderMesh();
	_renderMode &= ~(RenderMode::EDGES | RenderMode::NODES); // !!!

    UI::Outliner::Instance()->DeleteItem(_outlinerItem);
}

leda::nb::RatPolyMesh& ENT_Geometry::GetRatPolyMesh() { return _ratPolyMesh; }

void ENT_Geometry::Rotate(float ang, float x, float y, float z) {
	SYS::ScopedLock lock(_mtx);
    GetTransform().rotation = M::RotationOf(M::Mat4::RotateAxis(M::Normalize(M::Vector3(x, y, z)), ang)) * GetTransform().rotation;
}

void ENT_Geometry::HideOutline() {
    SYS::ScopedLock lock(_mtx);
    UI::Outliner::Instance()->HideItem(_outlinerItem);
}

void ENT_Geometry::Show() { 
	SYS::ScopedLock lock(_mtx);
	_isHidden = false; 
}

void ENT_Geometry::Hide() { 
	SYS::ScopedLock lock(_mtx);
	_isHidden = true; 
}

void ENT_Geometry::SetRenderMode(int flags) { 
	SYS::ScopedLock lock(_mtx);
	_renderMode = flags; 
}

void ENT_Geometry::SetRenderLayer(unsigned layer) { 
	SYS::ScopedLock lock(_mtx);
	_renderLayer = layer; 
}

void ENT_Geometry::SetShadingMode(ShadingMode::Enum mode) { 
	SYS::ScopedLock lock(_mtx);
	_shadingMode = mode; 
}

void ENT_Geometry::FrameUpdate() {
	SYS::ScopedLock lock(_mtx);
    M::Matrix4 tf = M::Mat4::FromRigidTransform(GetTransform().rotation, GetTransform().position);
    _nodes.Transform(world.GetCameraMatrix() * tf);
    _edgeRenderer->SetTransform(tf, world.GetCameraMatrix());
}

void ENT_Geometry::BuildRenderList() {
	SYS::ScopedLock lock(_mtx);
    _renderList.meshJobs.clear();

    if(_isHidden) return;

    if(RenderMode::FACES & _renderMode && NULL != _mesh) {
        R::meshMgr.GetMesh(_tfmesh).SetTransform(GetTransformationMatrix());

        R::MeshJob rjob;
        rjob.layer      = _renderLayer;
        rjob.fx         = "LitDirectional";
        rjob.material   = R::Material::White;
        rjob.tfmesh     = _tfmesh;
        rjob.primType   = 0;
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::NODES & _renderMode && ShadingMode::NICE == _shadingMode && !_nodes.IsEmpty()) {
	    _nodes.BuildRenderMesh();
        R::MeshJob rjob = _nodes.GetRenderJob();
        rjob.layer = _renderLayer;
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::EDGES & _renderMode && ShadingMode::NICE == _shadingMode && !_edgeRenderer->IsEmpty()) {
		_edgeRenderer->BuildRenderMesh();
        R::MeshJob rjob = _edgeRenderer->GetRenderJob();
        rjob.layer = _renderLayer;
        _renderList.meshJobs.push_back(rjob);
    }
}

} // namespace W