#include <Nubuck\math\sphere.h>
#include <Nubuck\math\intersections.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <UI\outliner\outliner.h>
#include <UI\userinterface.h>
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

static leda::edge UnmaskedSucc(const leda::nb::RatPolyMesh& G, leda::edge e) {
    // assumes NULL != e and at least one visible adjacent edge.
    // if thats not the case you deserve to crash... 

    const leda::edge eR = G.reversal(e);
    leda::edge it = eR;
    do {
        it = G.cyclic_adj_pred(it);
    } while(G.is_masked(it) && it != eR);

    return it;
}

void ENT_Geometry::RebuildRenderMesh() {
    _vertices.clear();
    _indices.clear();

    _isTransparent = false;

    if(0 == _ratPolyMesh.number_of_faces()) return;

    unsigned idxCnt = 0;

    leda::face f;
    forall_faces(f, _ratPolyMesh) {
        if(!_ratPolyMesh.is_visible(f)) continue;

        leda::edge e = _ratPolyMesh.first_face_edge(f);
        if(_ratPolyMesh.is_masked(e)) continue;

        leda::edge n = _ratPolyMesh.face_cycle_succ(e);
        const M::Vector3& p0 = _fpos[leda::source(e)->id()];
        const M::Vector3& p1 = _fpos[leda::target(e)->id()];
        const M::Vector3& p2 = _fpos[leda::target(n)->id()];
        const M::Vector3 normal = M::Normalize(M::Cross(p1 - p0, p2 - p0));

        R::Mesh::Vertex vert;
        vert.normal = normal;
        vert.color = _ratPolyMesh.color_of(f);
        vert.color.a *= _transparency;
        if(1.0f > vert.color.a) _isTransparent = true;

        leda::edge it = e;
        do {
            vert.position = _fpos[leda::source(it)->id()];
            _vertices.push_back(vert);
            _indices.push_back(idxCnt++);
            it = UnmaskedSucc(_ratPolyMesh, it);
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

void ENT_Geometry::Event_TransparencyChanged(const EV::Event& event) {
    const EV::Params_ENT_Geometry_TransparencyChanged& args = EV::def_ENT_Geometry_TransparencyChanged.GetArgs(event);
    SetTransparency(args.transparency);
}

void ENT_Geometry::Event_RenderModeChanged(const EV::Event& event) {
    const EV::Params_ENT_Geometry_RenderModeChanged& args = EV::def_ENT_Geometry_RenderModeChanged.GetArgs(event);
    std::cout << "RECEIVED!" << std::endl;
    SetRenderMode(args.renderMode);
}

void ENT_Geometry::Event_EdgeShadingChanged(const EV::Event& event) {
    const EV::Params_ENT_Geometry_EdgeShadingChanged& args = EV::def_ENT_Geometry_EdgeShadingChanged.GetArgs(event);
    SetShadingMode(args.shadingMode);
}

ENT_Geometry::ENT_Geometry() 
    : _outlinerItem(NULL)
    , _edgeRenderer(NULL)
    , _mesh(NULL)
    , _tfmesh(NULL)
    , _meshCompiled(true)
    , _isSolid(true)
    , _isHidden(false)
    , _renderMode(0)
    , _renderLayer(0)
    , _shadingMode(ShadingMode::NICE)
    , _transparency(1.0f)
    , _isTransparent(false)
{ 
    _edgeRenderer = &_cylinderEdges;
    // _edgeRenderer = &_lineEdges;

    _edgeRadius = 0.02f;
    _edgeColor = R::Color(0.3f, 0.3f, 0.3f);

    _outlinerItem = g_ui.GetOutliner().AddItem("", this);

    SetName("Mesh");

	AddEventHandler(EV::def_ENT_Geometry_EdgeRadiusChanged, this, &ENT_Geometry::Event_EdgeRadiusChanged);
	AddEventHandler(EV::def_ENT_Geometry_EdgeColorChanged, this, &ENT_Geometry::Event_EdgeColorChanged);
    AddEventHandler(EV::def_ENT_Geometry_TransparencyChanged, this, &ENT_Geometry::Event_TransparencyChanged);
    AddEventHandler(EV::def_ENT_Geometry_RenderModeChanged, this, &ENT_Geometry::Event_RenderModeChanged);
    AddEventHandler(EV::def_ENT_Geometry_EdgeShadingChanged, this, &ENT_Geometry::Event_EdgeShadingChanged);
}

bool ENT_Geometry::TraceVertices(const M::Ray& ray, float radius, std::vector<leda::node>& verts) {
    SYS::ScopedLock lock(_mtx);
    leda::node v;
    verts.clear();
    forall_nodes(v, _ratPolyMesh) {
        M::Vector3 pos = Transform(ToVector(_ratPolyMesh.position_of(v)));
        if(M::IS::Intersects(ray, M::Sphere(pos, radius)))
            verts.push_back(v);
    }
    return !verts.empty();
}

static bool elem(const std::vector<leda::node>& set, const leda::node x) {
    for(unsigned i = 0; i < set.size(); ++i)
        if(set[i] == x) return true;
    return false;
}

void ENT_Geometry::Select(const leda::node v) {
    SYS::ScopedLock lock(_mtx);
    if(!elem(_vertexSelection, v)) _vertexSelection.push_back(v);
}

std::vector<leda::node> ENT_Geometry::GetVertexSelection() const {
    SYS::ScopedLock lock(_mtx);
    return _vertexSelection;
}

void ENT_Geometry::ClearVertexSelection() {
    SYS::ScopedLock lock(_mtx);
    _vertexSelection.clear();
}

UI::OutlinerView* ENT_Geometry::CreateOutlinerView() {
    return new ENT_GeometryOutln(*this);
}

void ENT_Geometry::SetSolid(bool solid) {
    _isSolid = solid;
}

void ENT_Geometry::SetTransparency(float transparency) {
    SYS::ScopedLock lock(_mtx);
    if(_transparency != transparency) {
        for(unsigned i = 0; i < _meshDesc.numVertices; ++i)
            _meshDesc.vertices[i].color.a = transparency;
        R::meshMgr.GetMesh(_mesh).Invalidate(_meshDesc.vertices);
        _transparency = transparency;
        // TODO: this ignores alpha of vertices!
        _isTransparent = 1.0f > _transparency;
    }
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
    leda::edge_array<bool> visited(_ratPolyMesh, false);
	std::vector<R::EdgeRenderer::Edge> edges;
    R::EdgeRenderer::Edge re;
    leda::edge e;
    forall_edges(e, _ratPolyMesh) {
        if(!visited[e] && !_ratPolyMesh.is_masked(e)) {
            re.radius = _ratPolyMesh.radius_of(e);
            re.color = _ratPolyMesh.color_of(e);
            re.p0 = ToVector(_ratPolyMesh.position_of(leda::source(e)));
            re.p1 = ToVector(_ratPolyMesh.position_of(leda::target(e)));
            edges.push_back(re);
            visited[e] = visited[_ratPolyMesh.reversal(e)] = true;
        }
    }
    _edgeRenderer->Rebuild(edges);
}

void ENT_Geometry::Rebuild() {
	SYS::ScopedLock lock(_mtx);

    bool update = 0 < _ratPolyMesh.clear_update_flags();
    bool rebuild = _ratPolyMesh.needs_rebuild();

    if(!update && !rebuild) return; // nothing to do

    CacheFPos();
    RebuildRenderNodes();
    RebuildRenderEdges();
    RebuildRenderMesh();
    ComputeBoundingBox();

    if(rebuild) {
        _ratPolyMesh.cache_all();
    }
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
    g_ui.GetOutliner().SendToView(_outlinerItem, event);
}

void ENT_Geometry::SetEdgeColor(const R::Color& color) {
	SYS::ScopedLock lock(_mtx);
    _edgeColor = color;
    RebuildRenderEdges();

    EV::Params_ENT_Geometry_EdgeColorChanged args = { _edgeColor };
    EV::Event event = EV::def_ENT_Geometry_EdgeColorChanged.Create(args);
    g_ui.GetOutliner().SendToView(_outlinerItem, event);
}

M::Vector3 ENT_Geometry::GetLocalCenter() const { 
	SYS::ScopedLock lock(_mtx);
    return _bbox.min + 0.5f * (_bbox.max - _bbox.min); 
}

M::Vector3 ENT_Geometry::GetGlobalCenter() { 
	SYS::ScopedLock lock(_mtx);
    M::Vector3 localCenter = GetLocalCenter();
    return Transform(localCenter); 
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

void ENT_Geometry::SetName(const std::string& name) {
    Entity::SetName(name);
    g_ui.GetOutliner().SetItemName(_outlinerItem, QString::fromStdString(name));
}

void ENT_Geometry::OnDestroy() {
	SYS::ScopedLock lock(_mtx);
	_nodes.DestroyRenderMesh();
	_edgeRenderer->DestroyRenderMesh();
	_renderMode &= ~(RenderMode::EDGES | RenderMode::NODES); // !!!

    g_ui.GetOutliner().DeleteItem(_outlinerItem);
}

leda::nb::RatPolyMesh& ENT_Geometry::GetRatPolyMesh() { return _ratPolyMesh; }

const leda::nb::RatPolyMesh& ENT_Geometry::GetRatPolyMesh() const { return _ratPolyMesh; }

void ENT_Geometry::Rotate(float ang, float x, float y, float z) {
	SYS::ScopedLock lock(_mtx);
    GetTransform().rotation = M::RotationOf(M::Mat4::RotateAxis(M::Normalize(M::Vector3(x, y, z)), ang)) * GetTransform().rotation;
}

void ENT_Geometry::HideOutline() {
    SYS::ScopedLock lock(_mtx);
    g_ui.GetOutliner().HideItem(_outlinerItem);
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
    bool rebuild = false;
    _mtx.Lock();
    if(_shadingMode != mode) {
        if(ShadingMode::FAST == mode) _edgeRenderer = &_lineEdges;
        else _edgeRenderer = &_cylinderEdges; 
        _shadingMode = mode;
        rebuild = true;
    }
    _mtx.Unlock();

    if(rebuild) RebuildRenderEdges();
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

        if(_isTransparent) rjob.fx = "LitDirectionalTransparent";
        else rjob.fx = "LitDirectionalTwosided";

        rjob.layer      = _renderLayer;
        rjob.material   = R::Material::White;
        rjob.tfmesh     = _tfmesh;
        rjob.primType   = 0;
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::NODES & _renderMode && !_nodes.IsEmpty()) {
	    _nodes.BuildRenderMesh();
        R::MeshJob rjob = _nodes.GetRenderJob();
        rjob.layer = _renderLayer;
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::EDGES & _renderMode && !_edgeRenderer->IsEmpty()) {
		_edgeRenderer->BuildRenderMesh();
        R::MeshJob rjob = _edgeRenderer->GetRenderJob();
        rjob.layer = _renderLayer;
        _renderList.meshJobs.push_back(rjob);
    }
}

} // namespace W