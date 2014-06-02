#include <Nubuck\math\sphere.h>
#include <Nubuck\math\intersections.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <UI\outliner\outliner.h>
#include <UI\userinterface.h>
#include <renderer\metrics\metrics.h>
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

static R::Color selectedVertexColor     = R::Color::Yellow;
static R::Color unselectedVertexColor   = R::Color::Black;

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
    _vmap.clear();
    _faces.clear();
    _vertices.clear();
    _indices.clear();

    _isTransparent = false;

    if(0 == _ratPolyMesh.number_of_faces()) return;

    unsigned idxCnt = 0;

    _faces.resize(_ratPolyMesh.max_face_index() + 1);

    leda::face f;
    forall_faces(f, _ratPolyMesh) {
        if(!_ratPolyMesh.is_visible(f)) continue;

        leda::edge e = _ratPolyMesh.first_face_edge(f);
        if(_ratPolyMesh.is_masked(e)) continue;

        Face& face = _faces[f->id()];
        face.sz = 0;

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
            leda::node pv = leda::source(it);
            vert.position = _fpos[pv->id()];
            _vmap.push_back(pv);
            if(0 == face.sz++) face.idx = _vertices.size();
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

void ENT_Geometry::UpdateRenderMesh() {
    typedef leda::nb::RatPolyMesh::State state_t;
    for(unsigned i = 0; i < _vertices.size(); ++i) {
        leda::node pv = _vmap[i];
        if(state_t::GEOMETRY_CHANGED == _ratPolyMesh.state_of(pv)) {
            _vertices[i].position = _fpos[pv->id()];
            const unsigned vertSz = sizeof(R::Mesh::Vertex);
            const unsigned off = vertSz * (&_vertices[i] - &_vertices[0]);
            if(_mesh) R::meshMgr.GetMesh(_mesh).Invalidate(&_vertices[0], off, vertSz);
        }
    }

    leda::face f;
    forall_faces(f, _ratPolyMesh) {
        if(state_t::GEOMETRY_CHANGED == _ratPolyMesh.state_of(f)) {
            const Face& rf = _faces[f->id()];
            const M::Vector3& p0 = _vertices[rf.idx + 0].position;
            const M::Vector3& p1 = _vertices[rf.idx + 1].position;
            const M::Vector3& p2 = _vertices[rf.idx + 2].position;
            const M::Vector3 normal = M::Normalize(M::Cross(p1 - p0, p2 - p0));

            for(unsigned i = 0; i < rf.sz; ++i) _vertices[rf.idx + i].normal = normal;

            const unsigned vertSz = sizeof(R::Mesh::Vertex);
            const unsigned off = vertSz * (&_vertices[rf.idx] - &_vertices[0]);
            const unsigned sz = vertSz * rf.sz;
            if(_mesh) R::meshMgr.GetMesh(_mesh).Invalidate(&_vertices[0], off, sz);
        }
    }
}

void ENT_Geometry::DestroyRenderMesh() {
    if(_mesh) {
        R::meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
    if(_tfmesh) {
        R::meshMgr.Destroy(_tfmesh);
        _tfmesh = NULL;
    }
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
    _stylizedHiddenLines = args.showHiddenLines;
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
    , _stylizedHiddenLines(false)
    , _transparency(1.0f)
    , _isTransparent(false)
{
    _nodeRenderer = &_billboardNodes;

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
    verts.clear();
    M::Matrix4 objToWorld = GetObjectToWorldMatrix();
    leda::node v;
    forall_nodes(v, _ratPolyMesh) {
        M::Vector3 pos = M::Transform(objToWorld, ToVector(_ratPolyMesh.position_of(v)));
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
    if(!elem(_vertexSelection, v)) {
        _vertexSelection.push_back(v);

        if(editMode_t::VERTICES == world.GetEditMode().GetMode())
            _nodeRenderer->SetColor(v, selectedVertexColor);
    }
}

std::vector<leda::node> ENT_Geometry::GetVertexSelection() const {
    SYS::ScopedLock lock(_mtx);
    return _vertexSelection;
}

void ENT_Geometry::ClearVertexSelection() {
    SYS::ScopedLock lock(_mtx);

    if(editMode_t::VERTICES == world.GetEditMode().GetMode()) {
        for(unsigned i = 0; i < _vertexSelection.size(); ++i)
            _nodeRenderer->SetColor(_vertexSelection[i], unselectedVertexColor);
    }

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
        if(_mesh) R::meshMgr.GetMesh(_mesh).Invalidate(_meshDesc.vertices);
        _transparency = transparency;
        // TODO: this ignores alpha of vertices!
        _isTransparent = 1.0f > _transparency;
    }
}

bool ENT_Geometry::IsMeshCompiled() const { return _meshCompiled; }

void ENT_Geometry::RebuildRenderEdges() {
    leda::edge_array<bool> visited(_ratPolyMesh, false);
	std::vector<R::EdgeRenderer::Edge> edges;
    R::EdgeRenderer::Edge re;
    leda::edge e;
    forall_edges(e, _ratPolyMesh) {
        if(!visited[e] && !_ratPolyMesh.is_masked(e)) {
            re.radius = 2 * _ratPolyMesh.radius_of(e); // !!!
            re.color = _ratPolyMesh.color_of(e);
            re.v0 = leda::source(e);
            re.v1 = leda::target(e);
            re.p0 = ToVector(_ratPolyMesh.position_of(leda::source(e)));
            re.p1 = ToVector(_ratPolyMesh.position_of(leda::target(e)));
            edges.push_back(re);
            visited[e] = visited[_ratPolyMesh.reversal(e)] = true;
        }
    }
    _edgeRenderer->Rebuild(edges);
}

static int GetUpdateState(const leda::nb::RatPolyMesh& mesh) {
    leda::node v;
    leda::edge e;
    leda::face f;
    int state = leda::nb::RatPolyMesh::State::CACHED;
    forall_nodes(v, mesh) state = M::Max(state, mesh.state_of(v));
    forall_edges(e, mesh) state = M::Max(state, mesh.state_of(e));
    forall_faces(f, mesh) state = M::Max(state, mesh.state_of(f));
    return state;
}

void ENT_Geometry::Rebuild() {
    typedef leda::nb::RatPolyMesh::State state_t;

	SYS::ScopedLock lock(_mtx);

    int state = GetUpdateState(_ratPolyMesh);

    if(state_t::CACHED == state) return; // nothing to do

    CacheFPos();

    if(state_t::TOPOLOGY_CHANGED == state) {
        _ratPolyMesh.cache_all();
        _nodeRenderer->Rebuild(_ratPolyMesh, _fpos);
        RebuildRenderEdges();
        RebuildRenderMesh();

        // TODO. used to colorize vertices
        SetEditMode(world.GetEditMode().GetMode());
    } else {
        _nodeRenderer->Update(_ratPolyMesh, _fpos);
        _edgeRenderer->Update(_ratPolyMesh, _fpos);

        UpdateRenderMesh();
    }

    ComputeBoundingBox();
}

static leda::d3_rat_point ToRatPoint(const M::Vector3& v) {
    return leda::d3_rat_point(leda::d3_point(v.x, v.y, v.z));
}

void ENT_Geometry::ApplyTransformation() {
	SYS::ScopedLock lock(_mtx);

    M::Matrix4 objToWorld = GetObjectToWorldMatrix();
    leda::node n;
    forall_nodes(n, _ratPolyMesh) {
        M::Vector3 pos = M::Transform(objToWorld, ToVector(_ratPolyMesh.position_of(n)));
        _ratPolyMesh.set_position(n, ToRatPoint(pos));
    }

    Entity::SetPosition(M::Vector3::Zero);
    Entity::SetOrientation(M::Quat::Identity());
    Entity::SetScale(M::Vector3(1.0f, 1.0f, 1.0f));
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
    return M::Transform(GetObjectToWorldMatrix(), localCenter);
}

const M::Box& ENT_Geometry::GetBoundingBox() const { 
	SYS::ScopedLock lock(_mtx);
	return _bbox; 
}

M::Vector3 ENT_Geometry::GetPosition() const {
    return Entity::GetPosition();
}

void ENT_Geometry::SetPosition(const M::Vector3& position) {
    Entity::SetPosition(position);
}

void ENT_Geometry::CompileMesh() {
	SYS::ScopedLock lock(_mtx);

    if(_meshCompiled) return;

    if(NULL != _mesh) R::meshMgr.Destroy(_mesh);
    _mesh = R::meshMgr.Create(_meshDesc);

    if(NULL != _tfmesh) R::meshMgr.Destroy(_tfmesh);
    _tfmesh = R::meshMgr.Create(_mesh);
    R::meshMgr.GetMesh(_tfmesh).SetTransform(GetObjectToWorldMatrix());

    _meshCompiled = true;
}

void ENT_Geometry::SetName(const std::string& name) {
    Entity::SetName(name);
    g_ui.GetOutliner().SetItemName(_outlinerItem, QString::fromStdString(name));
}

void ENT_Geometry::OnDestroy() {
	SYS::ScopedLock lock(_mtx);
	_nodeRenderer->DestroyRenderMesh();
	_edgeRenderer->DestroyRenderMesh();
    DestroyRenderMesh();
	_renderMode &= ~(RenderMode::EDGES | RenderMode::NODES); // !!!

    g_ui.GetOutliner().DeleteItem(_outlinerItem);
    _outlinerItem = NULL;
}

leda::nb::RatPolyMesh& ENT_Geometry::GetRatPolyMesh() { return _ratPolyMesh; }

const leda::nb::RatPolyMesh& ENT_Geometry::GetRatPolyMesh() const { return _ratPolyMesh; }

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

    EV::Params_ENT_Geometry_RenderModeChanged args = { _renderMode };
    EV::Event event = EV::def_ENT_Geometry_RenderModeChanged.Create(args);
    g_ui.GetOutliner().SendToView(_outlinerItem, event);
}

void ENT_Geometry::SetRenderLayer(unsigned layer) { 
	SYS::ScopedLock lock(_mtx);
	_renderLayer = layer; 
}

void ENT_Geometry::SetShadingMode(ShadingMode::Enum mode) { 
    bool rebuild = false;
    _mtx.Lock();
    if(_shadingMode != mode) {
        switch(mode) {
        case ShadingMode::FAST:
            _nodeRenderer = &_billboardNodes;
            _edgeRenderer = &_lineEdges;
            break;
        case ShadingMode::NICE:
            _nodeRenderer = &_billboardNodes;
            _edgeRenderer = &_cylinderEdges;
            break;
        case ShadingMode::LINES:
            _nodeRenderer = &_pointNodes;
            _edgeRenderer = &_glLineEdges;
            break;
        case ShadingMode::NICE_BILLBOARDS:
            _nodeRenderer = &_pointNodes;
            _edgeRenderer = &_glLineEdges;
            break;
        default:
            assert(0 && "ENT_Geometry::SetShadingMode(): unkown shading mode");
        };
        _shadingMode = mode;
        rebuild = true;
    }
    _mtx.Unlock();

    if(rebuild) {
        _nodeRenderer->Rebuild(_ratPolyMesh, _fpos);
        RebuildRenderEdges();
    }
}

void ENT_Geometry::SetEditMode(editMode_t::Enum mode) {
	SYS::ScopedLock lock(_mtx);

    if(editMode_t::VERTICES == mode) {
        leda::node pv;
        forall_nodes(pv, _ratPolyMesh) {
            _nodeRenderer->SetColor(pv, unselectedVertexColor);
        }

        for(unsigned i = 0; i < _vertexSelection.size(); ++i)
            _nodeRenderer->SetColor(_vertexSelection[i], selectedVertexColor);
    }

    if(editMode_t::OBJECTS == mode) {
        leda::node pv;
        forall_nodes(pv, _ratPolyMesh) {
            _nodeRenderer->SetColor(pv, _ratPolyMesh.color_of(pv));
        }
    }
}

void ENT_Geometry::FrameUpdate() {
    static SYS::Timer timer;

	SYS::ScopedLock lock(_mtx);
    M::Matrix4 tf = M::Mat4::ExpandedTR(Entity::GetPosition(), Entity::GetOrientation());
    _nodeRenderer->Transform(tf);

    timer.Start();
    _edgeRenderer->SetTransform(tf, world.GetCameraMatrix());
    R::metrics.frame.edgeRendererSetTransformAccu += timer.Stop();
}

void ENT_Geometry::BuildRenderList() {
	SYS::ScopedLock lock(_mtx);
    _renderList.meshJobs.clear();

    if(_isHidden) return;

    if(RenderMode::FACES & _renderMode && NULL != _mesh) {
        R::meshMgr.GetMesh(_tfmesh).SetTransform(GetObjectToWorldMatrix());

        R::MeshJob rjob;

        R::Material mat = R::Material::White;
        mat.isTransparent = _isTransparent;

        rjob.fx = "LitDirectionalTwosided";
        rjob.layer      = _renderLayer;
        rjob.material   = mat;
        rjob.tfmesh     = _tfmesh;
        rjob.primType   = 0;
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::NODES & _renderMode && !_nodeRenderer->IsEmpty()) {
	    _nodeRenderer->BuildRenderMesh();
        R::MeshJob rjob = _nodeRenderer->GetRenderJob();
        rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
        if(ShadingMode::FAST == _shadingMode) {
            rjob.fx = "FastNodeBillboard";
        }
        if(ShadingMode::NICE_BILLBOARDS == _shadingMode) {
            rjob.fx = "NodeBillboardGS";
            rjob.layer = R::Renderer::Layers::GEOMETRY_0_USE_DEPTH_0;
        }
        _renderList.meshJobs.push_back(rjob);
    }

    if(RenderMode::EDGES & _renderMode && !_edgeRenderer->IsEmpty()) {
		_edgeRenderer->BuildRenderMesh();
        R::MeshJob rjob = _edgeRenderer->GetRenderJob();
        rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
        if(ShadingMode::LINES == _shadingMode) {
            if(_stylizedHiddenLines) {
                rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
                rjob.fx = "UnlitThickLinesStippled";
            }
        }
        if(ShadingMode::NICE_BILLBOARDS == _shadingMode) {
            rjob.fx = "EdgeLineBillboardGS";
            rjob.layer = R::Renderer::Layers::GEOMETRY_0_USE_DEPTH_0;
        }
        _renderList.meshJobs.push_back(rjob);
    }
}

} // namespace W