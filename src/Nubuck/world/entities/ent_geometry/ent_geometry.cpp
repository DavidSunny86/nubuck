#include <Nubuck\math\sphere.h>
#include <Nubuck\math\intersections.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <Nubuck\animation\animator.h>
#include <UI\outliner\outliner.h>
#include <UI\userinterface.h>
#include <renderer\metrics\metrics.h>
#include <renderer\texfont\texfont.h>
#include <renderer\mesh\quad\quad.h>
#include <world\world_events.h>
#include <world\world.h>
#include <operators\operators.h>
#include "ent_geometry_outln.h"
#include "ent_geometry.h"

static COM::Config::Variable<float>    cvar_faceCurveSpeed("faceSpeed", 0.2f);
static COM::Config::Variable<float>    cvar_faceCurveDecalSize("faceDecalSize", 0.2f);
static COM::Config::Variable<float>    cvar_faceCurveSpacing("faceSpacing", 0.4f);
static COM::Config::Variable<float>    cvar_faceCurveCurvature("faceCurvature", 0.2f);

namespace {

M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

} // unnamed namespace

namespace W {

void ENT_Geometry::DecalMesh::Rebuild(const std::vector<Curve>& curves) {
    m_vertices.clear();
    m_indices.clear();

    if(curves.empty()) {
        return;
    }

    M::Vector2 texCoords[] = {
        M::Vector2(-1.0f, -1.0f),
        M::Vector2( 1.0f, -1.0f),
        M::Vector2( 1.0f,  1.0f),
        M::Vector2(-1.0f,  1.0f)
    };

    R::Mesh::Index indices[] = { 0, 1, 2, 0, 2, 3 };

    for(unsigned i = 0; i < curves.size(); ++i) {
        for(unsigned j = 0; j < curves[i].curve.decalPos.size(); ++j) {
            // emit quad indices
            unsigned baseIdx = m_vertices.size();
            for(int k = 0; k < 6; ++k) {
                m_indices.push_back(baseIdx + indices[k]);
            }
            m_indices.push_back(R::Mesh::RESTART_INDEX);

            // emit quad vertices

            const M::Vector3& position = curves[i].curve.decalPos[j];
            const M::Matrix3& localToWorld = curves[i].localToWorld;

            R::Mesh::Vertex vertex;
            for(int k = 0; k < 4; ++k) {
                vertex.position = position;
                vertex.normal.x = cvar_faceCurveDecalSize;
                vertex.texCoords = texCoords[k];
                vertex.color = curves[i].color;
                vertex.A[0] = M::Vector3(localToWorld.m00, localToWorld.m10, localToWorld.m20);
                vertex.A[1] = M::Vector3(localToWorld.m01, localToWorld.m11, localToWorld.m21);
                vertex.A[2] = M::Vector3(localToWorld.m02, localToWorld.m12, localToWorld.m22);
                m_vertices.push_back(vertex);
            }
        }
    }

    bool rebuildMesh = !m_mesh || R::meshMgr.GetMesh(m_mesh).GetVertices().size() < m_vertices.size();
    if(true || rebuildMesh) { // !!!
        if(m_mesh) R::meshMgr.Destroy(m_mesh);
        if(m_tfmesh) R::meshMgr.Destroy(m_tfmesh);

        R::Mesh::Desc meshDesc;
        meshDesc.indices = &m_indices[0];
        meshDesc.numIndices = m_indices.size();
        meshDesc.vertices = &m_vertices[0];
        meshDesc.numVertices = m_vertices.size();
        meshDesc.primType = GL_TRIANGLES;

        m_mesh = R::meshMgr.Create(meshDesc);
        m_tfmesh = R::meshMgr.Create(m_mesh);
    } else {
        R::meshMgr.GetMesh(m_mesh).Invalidate(&m_vertices[0], 0, sizeof(R::Mesh::Vertex) * m_vertices.size());
        R::meshMgr.GetMesh(m_mesh).Invalidate(&m_indices[0], m_indices.size());
    }
}

void ENT_Geometry::DecalMesh::Render(R::RenderList& renderList) {
    if(!m_vertices.empty()) {
        R::Material mat = R::Material::White;

        R::Texture* tex = R::TextureManager::Instance().Get(common.BaseDir() + "Textures\\circle.tga").Raw();
        mat.SetUniformBinding("decalTex", tex);

        R::MeshJob mjob;
        mjob.fx = "Decals";
        mjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
        mjob.material = mat;
        mjob.primType = 0;
        mjob.tfmesh = m_tfmesh;

        renderList.meshJobs.push_back(mjob);
    }
}

void ENT_Geometry::ForceRebuild() {
    OP::g_operators.InvokeAction(ev_w_rebuildAll.Tag(), OP::Operators::InvokationMode::ALWAYS);
    _forceRebuild = true;
}

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

    _meshCompiled = false;

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

        const R::Color& pc = _ratPolyMesh.pattern_of(f);
        vert.A[0] = M::Vector3(pc.r, pc.g, pc.b);
        vert.A[1].x = pc.a;

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

    _isTransparent = false;

    leda::face f;
    forall_faces(f, _ratPolyMesh) {
        if(state_t::GEOMETRY_CHANGED == _ratPolyMesh.state_of(f)) {
            const Face& rf = _faces[f->id()];
            const M::Vector3& p0 = _vertices[rf.idx + 0].position;
            const M::Vector3& p1 = _vertices[rf.idx + 1].position;
            const M::Vector3& p2 = _vertices[rf.idx + 2].position;
            const M::Vector3 normal = M::Normalize(M::Cross(p1 - p0, p2 - p0));

            const R::Color& color = _ratPolyMesh.color_of(f);
            if(1.0f > color.a) _isTransparent = true;

            for(unsigned i = 0; i < rf.sz; ++i) {
                _vertices[rf.idx + i].normal = normal;
                _vertices[rf.idx + i].color = color;
            }

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

M::Box ENT_Geometry::GetBoundingBox() const {
    M::Box bbox;
    bbox.min = bbox.max = M::Vector3::Zero;
    leda::node v;
    forall_nodes(v, _ratPolyMesh) {
        M::Vector3 p = ToVector(_ratPolyMesh.position_of(v));
        bbox.min.x = M::Min(bbox.min.x, p.x);
        bbox.min.y = M::Min(bbox.min.y, p.y);
        bbox.min.z = M::Min(bbox.min.z, p.z);
        bbox.max.x = M::Max(bbox.max.x, p.x);
        bbox.max.y = M::Max(bbox.max.y, p.y);
        bbox.max.z = M::Max(bbox.max.z, p.z);
    }
    if(M::AlmostEqual(0.0f, M::SquaredDistance(bbox.min, bbox.max))) {
        // set empty bbox to some minimum size
        bbox.min = -bbox.max = M::Vector3(0.1f, 0.1f, 0.1f);
    }
    return bbox;
}

void ENT_Geometry::Event_ShowVertexLabels(const EV::Arg<bool>& event) {
    _showVertexLabels = event.value;
    ForceRebuild();

    g_ui.GetOutliner().SendToView(_outlinerItem, event);
}

void ENT_Geometry::Event_XrayVertexLabels(const EV::Arg<bool>& event) {
    _xrayVertexLabels = event.value;
    ForceRebuild();

    g_ui.GetOutliner().SendToView(_outlinerItem, event);
}

void ENT_Geometry::Event_SetVertexLabelSize(const EV::Arg<float>& event) {
    _vertexLabelSize = event.value;
    ForceRebuild();

    g_ui.GetOutliner().SendToView(_outlinerItem, event);
}

void ENT_Geometry::Event_VertexScaleChanged(const EV::Arg<float>& event) {
    SYS::ScopedLock lock(_mtx);
    _vertexScale = event.value;
    ForceRebuild();
}

void ENT_Geometry::Event_EdgeScaleChanged(const EV::Arg<float>& event) {
	SYS::ScopedLock lock(_mtx);
    _edgeScale = event.value;
    ForceRebuild();
}

void ENT_Geometry::Event_EdgeTintChanged(const EV::Arg<R::Color>& event) {
	SYS::ScopedLock lock(_mtx);
    _edgeTint = event.value;
    ForceRebuild();
}

void ENT_Geometry::Event_TransparencyChanged(const EV::Arg<float>& event) {
    SetTransparency(event.value);
}

void ENT_Geometry::Event_RenderModeChanged(const RenderModeEvent& event) {
    std::cout << "RECEIVED!" << std::endl;
    if(_renderMode != event.renderMode) SetRenderMode(event.renderMode);
    _showWireframe = event.showWireframe;
    _showNormals = event.showNormals;
}

void ENT_Geometry::Event_EdgeShadingChanged(const EdgeShadingEvent& event) {
    _stylizedHiddenLines = event.showHiddenLines;
    SetShadingMode(ShadingMode(event.shadingMode));
}

ENT_Geometry::ENT_Geometry()
    : _outlinerItem(NULL)
    , _edgeRenderer(NULL)
    , _showVertexLabels(false)
    , _xrayVertexLabels(false)
    , _vertexLabelSize(1.0f)
    , _mesh(NULL)
    , _tfmesh(NULL)
    , _meshCompiled(true)
    , _forceRebuild(false)
    , _isHidden(false)
    , _renderMode(0)
    , _renderLayer(0)
    , _shadingMode(NB::SM_NICE)
    , _pattern(NB::PATTERN_NONE)
    , _patternColor(R::Color::White)
    , _stylizedHiddenLines(false)
    , _showWireframe(false)
    , _showNormals(false)
    , _transparency(1.0f)
    , _isTransparent(false)
    , _anims(NULL)
{
    _nodeRenderer = &_billboardNodes;

    _edgeRenderer = &_cylinderEdges;
    // _edgeRenderer = &_lineEdges;

    _vertexScale    = 1.0f;
    _edgeScale      = 1.0f;
    _edgeTint      = R::Color(0.3f, 0.3f, 0.3f);

    _outlinerItem = g_ui.GetOutliner().AddItem("", this);

    SetName("Mesh");

    AddEventHandler(ev_geom_showVertexLabels, this, &ENT_Geometry::Event_ShowVertexLabels);
    AddEventHandler(ev_geom_setVertexLabelSize, this, &ENT_Geometry::Event_SetVertexLabelSize);
    AddEventHandler(ev_geom_xrayVertexLabels, this, &ENT_Geometry::Event_XrayVertexLabels);
    AddEventHandler(ev_geom_vertexScaleChanged, this, &ENT_Geometry::Event_VertexScaleChanged);
	AddEventHandler(ev_geom_edgeScaleChanged, this, &ENT_Geometry::Event_EdgeScaleChanged);
	AddEventHandler(ev_geom_edgeTintChanged, this, &ENT_Geometry::Event_EdgeTintChanged);
    AddEventHandler(ev_geom_transparencyChanged, this, &ENT_Geometry::Event_TransparencyChanged);
    AddEventHandler(ev_geom_renderModeChanged, this, &ENT_Geometry::Event_RenderModeChanged);
    AddEventHandler(ev_geom_edgeShadingChanged, this, &ENT_Geometry::Event_EdgeShadingChanged);
}

int GetUpdateState(const leda::nb::RatPolyMesh& mesh);

bool ENT_Geometry::IsDirty() const {
    return GetUpdateState(_ratPolyMesh);
}

bool ENT_Geometry::TraceVertices(const M::Ray& ray, float radius, std::vector<VertexHit>& hits) {
    SYS::ScopedLock lock(_mtx);
    hits.clear();
    M::Matrix4 objToWorld = GetObjectToWorldMatrix();
    M::IS::Info info;
    leda::node v;
    forall_nodes(v, _ratPolyMesh) {
        M::Vector3 pos = M::Transform(objToWorld, ToVector(_ratPolyMesh.position_of(v)));
        if(M::IS::Intersects(ray, M::Sphere(pos, radius), &info))
            hits.push_back(VertexHit(v, info.distance));
    }
    return !hits.empty();
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
    }
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
            re.pe = e;
            re.radius = 2 * _ratPolyMesh.radius_of(e) * _edgeScale; // !!!
            re.color0 = _ratPolyMesh.color_of(e);
            re.color1 = _ratPolyMesh.color_of(_ratPolyMesh.reversal(e));
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

static M::Vector2 FlipY(const M::Vector2& v) {
    return M::Vector2(v.x, -v.y);
}

void ENT_Geometry::RebuildVertexLabels() {
    if(!_showVertexLabels) return; // nothing to do

    const std::string filename = common.BaseDir() + "Textures\\Fonts\\consola.ttf_sdf.txt";
    const R::TexFont& texFont = R::FindTexFont(R::TF_Type::SDFont, filename);

    _vertexLabels.Begin(texFont);

    char label[100] = { '\0' };

    leda::node v;
    forall_nodes(v, _ratPolyMesh) {
        itoa(v->id(), label, 10);
        unsigned sidx = _vertexLabels.AddString(texFont, label, 'A', _vertexLabelSize, R::Color(1.0f, 1.0f, 1.0f, 0.85f));

        const M::Vector2& lowerLeft = _vertexLabels.GetLowerLeft(sidx);
        const M::Vector2& upperRight = _vertexLabels.GetUpperRight(sidx);
        const M::Vector2 upperLeft(lowerLeft.x, upperRight.y);
        const M::Vector2 off2 = upperLeft + 0.5f * FlipY(upperRight - lowerLeft);
        const M::Vector3 off3(-off2.x, -off2.y, 0.0f);

        _vertexLabels.SetOriginAndDepthProxy(sidx, _fpos[v->id()] + off3, _fpos[v->id()]);
    }

    _vertexLabels.End();
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

    // HACK: PolyMesh::force_rebuild does not affect newly-empty meshes,
    // so we always rebuild meshes with no vertices
    _forceRebuild = _forceRebuild || 0 == _ratPolyMesh.number_of_nodes();

    if(!_forceRebuild && state_t::CACHED == state) return; // nothing to do

    CacheFPos();

    if(_forceRebuild || state_t::TOPOLOGY_CHANGED == state) {
        _ratPolyMesh.cache_all();
        _nodeRenderer->Rebuild(_ratPolyMesh, _fpos, _vertexScale);
        RebuildRenderEdges();
        RebuildRenderMesh();

        _forceRebuild = false;
    } else {
        _nodeRenderer->Update(_ratPolyMesh, _fpos, _vertexScale);
        _edgeRenderer->Update(_ratPolyMesh, _fpos);

        UpdateRenderMesh();

        _ratPolyMesh.cache_all();
    }

    RebuildVertexLabels();
    RebuildCurves();
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

float ENT_Geometry::GetVertexScale() const {
    return _vertexScale;
}

float ENT_Geometry::GetEdgeScale() const {
	SYS::ScopedLock lock(_mtx);
    return _edgeScale;
}

R::Color ENT_Geometry::GetEdgeTint() const {
	SYS::ScopedLock lock(_mtx);
    return _edgeTint;
}

void ENT_Geometry::SetVertexScale(float vertexScale) {
    SYS::ScopedLock lock(_mtx);
    _vertexScale = vertexScale;
    _nodeRenderer->Rebuild(_ratPolyMesh, _fpos, _vertexScale);

    g_ui.GetOutliner().SendToView(_outlinerItem,
        ev_geom_vertexScaleChanged.Tag(_vertexScale));
}

void ENT_Geometry::SetEdgeScale(float edgeScale) {
	SYS::ScopedLock lock(_mtx);
    _edgeScale = edgeScale;
    RebuildRenderEdges();

    g_ui.GetOutliner().SendToView(_outlinerItem,
        ev_geom_edgeScaleChanged.Tag(_edgeScale));
}

void ENT_Geometry::SetEdgeTint(const R::Color& color) {
	SYS::ScopedLock lock(_mtx);
    _edgeTint = color;
    RebuildRenderEdges();

    g_ui.GetOutliner().SendToView(_outlinerItem,
        ev_geom_edgeTintChanged.Tag(_edgeTint));
}

M::Vector3 ENT_Geometry::GetPosition() const {
    return Entity::GetPosition();
}

void ENT_Geometry::SetPosition(const M::Vector3& position) {
    Entity::SetPosition(position);

    SetEntityVectorEvent event;
    NB::Point3 ratPos = ToRatPoint(GetPosition());
    event.m_entityID = GetID();
    event.m_vector[0] = ratPos.xcoord();
    event.m_vector[1] = ratPos.ycoord();
    event.m_vector[2] = ratPos.zcoord();

    g_ui.GetOutliner().SendToView(_outlinerItem,
        ev_ent_setPosition.Tag(event));
}

void ENT_Geometry::SetScale(const M::Vector3& scale) {
    Entity::SetScale(scale);
}

void ENT_Geometry::CompileMesh() {
	SYS::ScopedLock lock(_mtx);

    if(_meshCompiled) return;

    DestroyRenderMesh();

    if(_faces.size()) {
        _mesh = R::meshMgr.Create(_meshDesc);
        _tfmesh = R::meshMgr.Create(_mesh);
        R::meshMgr.GetMesh(_tfmesh).SetTransform(GetObjectToWorldMatrix());
    }

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
	_renderMode &= ~(NB::RM_EDGES | NB::RM_NODES); // !!!

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

NB::ShadingMode ENT_Geometry::GetShadingMode() const {
    return _shadingMode;
}

bool ENT_Geometry::StylizedHiddenLinesEnabled() const {
    return _stylizedHiddenLines;
}

void ENT_Geometry::SetRenderMode(int flags) {
	SYS::ScopedLock lock(_mtx);
	_renderMode = flags;

    // TODO: wireframe, normals
    RenderModeEvent event;
    event.renderMode = _renderMode;
    g_ui.GetOutliner().SendToView(_outlinerItem, ev_geom_renderModeChanged.Tag(event));
}

void ENT_Geometry::SetRenderLayer(unsigned layer) {
	SYS::ScopedLock lock(_mtx);
	_renderLayer = layer;
}

void ENT_Geometry::SetShadingMode(ShadingMode mode) {
    _mtx.Lock();
    if(_shadingMode != mode) {
        switch(mode) {
        case NB::SM_FAST:
            _nodeRenderer = &_billboardNodes;
            _edgeRenderer = &_lineEdges;
            break;
        case NB::SM_NICE:
            _nodeRenderer = &_billboardNodes;
            _edgeRenderer = &_cylinderEdges;
            break;
        case NB::SM_LINES:
            _nodeRenderer = &_pointNodes;
            _edgeRenderer = &_glLineEdges;
            break;
        case NB::SM_NICE_BILLBOARDS:
            _nodeRenderer = &_pointNodes;
            _edgeRenderer = &_glLineEdges;
            break;
        default:
            assert(0 && "ENT_Geometry::SetShadingMode(): unkown shading mode");
        };
        _shadingMode = mode;
        ForceRebuild();

        EdgeShadingEvent event;
        event.shadingMode = _shadingMode;
        event.showHiddenLines = _stylizedHiddenLines;
        g_ui.GetOutliner().SendToView(_outlinerItem, ev_geom_edgeShadingChanged.Tag(event));
    }
    _mtx.Unlock();
}

void ENT_Geometry::SetPattern(Pattern pattern) {
    _pattern = pattern;
}

void ENT_Geometry::SetPatternColor(const R::Color& color) {
    _patternColor = color;
}

void ENT_Geometry::FrameUpdate(float secsPassed) {
    static SYS::Timer timer;

	SYS::ScopedLock lock(_mtx);
    // FIXME: this way the bounding boxes of nodes, edges get scaled as
    // well, which is not what we want
    M::Matrix4 tf = GetObjectToWorldMatrix();
    _nodeRenderer->Transform(tf);

    timer.Start();
    _edgeRenderer->SetTransform(tf, world.GetCameraMatrix());
    R::metrics.frame.edgeRendererSetTransformAccu += timer.Stop();

    // rebuild curves, if necessary

    if(cvar_faceCurveCurvature.IsDirty()) {
        RebuildCurves();
    }

    bool rebuildDecals = cvar_faceCurveSpacing.IsDirty() || cvar_faceCurveDecalSize.IsDirty();

    // animate curves
    if(0.0f < cvar_faceCurveSpeed) {
        rebuildDecals = true;
        for(unsigned i = 0; i < _curves.size(); ++i) {
            _curves[i].time += cvar_faceCurveSpeed * secsPassed;
        }
    }

    if(rebuildDecals) {
        for(unsigned i = 0; i < _curves.size(); ++i) {
            ComputeCurveDecals(_curves[i]);
        }
    }

    cvar_faceCurveDecalSize.ClearDirtyFlag();
    cvar_faceCurveCurvature.ClearDirtyFlag();
    cvar_faceCurveSpacing.ClearDirtyFlag();
}

void ENT_Geometry::BuildRenderList() {
	SYS::ScopedLock lock(_mtx);
    _renderList.meshJobs.clear();

    if(_isHidden) return;

    const int transparencyMode = cvar_r_transparencyMode;

    if(NB::RM_FACES & _renderMode && NULL != _mesh) {
        R::meshMgr.GetMesh(_tfmesh).SetTransform(GetObjectToWorldMatrix());

        R::MeshJob rjob;

        R::Material mat = R::Material::White;
        mat.SetUniformBinding("patternColor", R::Color(0.0f, 0.0f, 0.0f, 0.0f));
        mat.SetUniformBinding("patternTex", NULL);

        rjob.material   = mat;
        rjob.tfmesh     = _tfmesh;
        rjob.primType   = 0;

        if(_showWireframe) {
            rjob.fx = "GenericWireframe";
            rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
            _renderList.meshJobs.push_back(rjob);
        }

        if(_showNormals) {
            rjob.fx = "GenericNormals";
            rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
            _renderList.meshJobs.push_back(rjob);
        }

        if(NB::PATTERN_NONE != _pattern) {
            const char* patternTextures[] = {
                "", /* none */
                "pattern_checker.tga",
                "pattern_dots.tga",
                "pattern_lines.tga"
            };

            R::Texture* patternTex = R::TextureManager::Instance().Get(common.BaseDir() + "Textures\\" + patternTextures[_pattern]).Raw();
            rjob.material.SetUniformBinding("patternColor", _patternColor);
            rjob.material.SetUniformBinding("patternTex", patternTex);
        }

        rjob.fx     = "DepthOnly";
        rjob.layer  = R::Renderer::Layers::GEOMETRY_0_SPINE_0;
        _renderList.meshJobs.push_back(rjob);

        if(_isTransparent) {
            rjob.fx     = "DepthOnly";

            rjob.layer  = R::Renderer::Layers::GEOMETRY_0_DEPTH_ONLY;
            _renderList.meshJobs.push_back(rjob);

            if(R::TransparencyMode::BACKFACES_FRONTFACES == transparencyMode) {
                rjob.fx         = "LitDirectionalTransparent";
                rjob.layer      = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
                _renderList.meshJobs.push_back(rjob);
            } else if(R::TransparencyMode::SORT_TRIANGLES == transparencyMode) {
                rjob.fx         = "LitDirectionalTransparent";
                rjob.layer      = R::Renderer::Layers::GEOMETRY_0_TRANSPARENT_SORTED;
                _renderList.meshJobs.push_back(rjob);
            } else if(R::TransparencyMode::DEPTH_PEELING == transparencyMode) {
                rjob.layer      = R::Renderer::Layers::GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_0;
                rjob.fx         = "LitDirectionalTwosidedPremulA";
                _renderList.meshJobs.push_back(rjob);

                rjob.layer      = R::Renderer::Layers::GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_N;
                rjob.fx         = "DepthPeel";
                _renderList.meshJobs.push_back(rjob);
            } else {
                assert(0 && "unknown transparency mode");
            }
        } else {
            rjob.fx         = "LitDirectionalTwosided";
            rjob.layer      = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
            _renderList.meshJobs.push_back(rjob);
        }

        // curves

        _decalMesh.Rebuild(_curves);
        _decalMesh.Render(_renderList);
    }

    if(NB::RM_NODES & _renderMode && !_nodeRenderer->IsEmpty()) {
	    _nodeRenderer->BuildRenderMesh();
        R::MeshJob rjob = _nodeRenderer->GetRenderJob();
        rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
        if(NB::SM_FAST == _shadingMode) {
            rjob.fx = "FastNodeBillboard";

            R::Material mat = R::Material::White;
            mat.SetUniformBinding("patternColor", R::Color(0.0f, 0.0f, 0.0f, 0.0f));
            mat.SetUniformBinding("patternTex", NULL);

            rjob.material = mat;
        }
        if(NB::SM_NICE_BILLBOARDS == _shadingMode) {
            if(R::TransparencyMode::DEPTH_PEELING == transparencyMode) {
                rjob.fx     = "NodeBillboardGSDP";
                rjob.layer  = R::Renderer::Layers::GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_USE_DEPTH;
                _renderList.meshJobs.push_back(rjob);
            }

            // render depth-only pass of nodes, so that grid and bboxes intersect properly
            rjob.fx     = "NodeBillboardGSDO";
            rjob.layer  = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
            _renderList.meshJobs.push_back(rjob);

            rjob.fx = "NodeBillboardGS";
            rjob.layer = R::Renderer::Layers::GEOMETRY_0_USE_SPINE_0;
        }
        _renderList.meshJobs.push_back(rjob);
    }

    if(NB::RM_EDGES & _renderMode && !_edgeRenderer->IsEmpty()) {
		_edgeRenderer->BuildRenderMesh();
        R::MeshJob rjob = _edgeRenderer->GetRenderJob();
        rjob.material.SetDiffuseColor(_edgeTint);
        rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
        if(NB::SM_LINES == _shadingMode) {
            rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
            _renderList.meshJobs.push_back(rjob);

            rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_2;

            if(_stylizedHiddenLines) {
                rjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
                rjob.fx = "UnlitThickLinesStippled";
            }
        }
        if(NB::SM_NICE_BILLBOARDS == _shadingMode) {
            if(R::TransparencyMode::DEPTH_PEELING == transparencyMode) {
                rjob.fx     = "EdgeLineBillboardGSDP";
                rjob.layer  = R::Renderer::Layers::GEOMETRY_0_TRANSPARENT_DEPTH_PEELING_USE_DEPTH;
                _renderList.meshJobs.push_back(rjob);
            }

            // render depth-only pass of edges, so that grid and bboxes intersect properly
            rjob.fx     = "EdgeLineBillboardGSDO";
            rjob.layer  = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
            _renderList.meshJobs.push_back(rjob);

            rjob.fx = "EdgeLineBillboardGS";

            if(cvar_r_smoothEdges) {
                rjob.fx = "EdgeLineBillboardGSSE";
            }

            rjob.layer = R::Renderer::Layers::GEOMETRY_0_USE_SPINE_0;
        }
        _renderList.meshJobs.push_back(rjob);
    }

    if(_showVertexLabels) {
        _vertexLabels.GetRenderJobsAlt(GetObjectToWorldMatrix(), _renderList, _xrayVertexLabels);
    }
}

void ENT_Geometry::AttachAnimation(A::Animation* anim) {
    anim->subjectLink.next = _anims;
    _anims = anim;
}

static inline M::Vector2 ProjXY(const M::Vector3& v) {
    return M::Vector2(v.x, v.y);
}

void ENT_Geometry::ComputeCurveDecals(Curve& cv) {
    // avoid overlapping decals
    float w = cvar_faceCurveDecalSize + cvar_faceCurveSpacing;
    int n = (int)(cv.curve.length / w);
    float def = cv.curve.length - n * w;
    w += def / n;

    std::vector<M::Vector2> decalPos2;
    cv.curve.decalPos.clear();
    R::SampleEquidistantPoints(cv.curve, cv.time, w, decalPos2);
    for(unsigned i = 0; i < decalPos2.size(); ++i) {
        M::Vector3 p = M::Transform(cv.localToWorld, M::Vector3(decalPos2[i].x, decalPos2[i].y, 0.0f)) + cv.origin;
        const float eps = 0.001f; // resolves z-fighting of faces and hull
        p += eps * cv.normal;
        cv.curve.decalPos.push_back(p);
    }
}


void ENT_Geometry::RebuildCurve(Curve& cv) {
    COM_assert(cv.faceIdx < _faces.size());
    const Face& face = _faces[cv.faceIdx];

    // create local copy of shrinked points
    std::vector<M::Vector3> points;
    points.reserve(face.sz);
    M::Vector3 center = M::Vector3::Zero;
    for(unsigned i = 0; i < face.sz; ++i) {
        const M::Vector3& p = _vertices[face.idx + i].position;
        center += p;
        points.push_back(p);
    }
    center /= points.size();
    const float f = 0.2f;
    for(unsigned i = 0; i < points.size(); ++i)
        points[i] = points[i] - f * (points[i] - center);

    const M::Vector3& p0 = points[0];
    const M::Vector3& p1 = points[1];
    const M::Vector3& p2 = points[2];

    M::Vector3 v0 = M::Normalize(p1 - p0);
    M::Vector3 v1 = M::Normalize(p2 - p0);
    M::Vector3 v2 = M::Normalize(M::Cross(v0, v1));

    M::Matrix3 M(M::Mat3::FromColumns(v0, v1, v2));
    if(M::AlmostEqual(0.0f, M::Det(M))) common.printf("ERROR - ENT_Geometry::RebuildCurve: matrix M is not invertable.\n");
    M::Orthonormalize(M);
    M::Matrix3 invM(M::Inverse(M)); // TODO transpose

    cv.localToWorld = M;
    cv.origin = p0;
    cv.normal = v2;

    cv.curve = R::PolyBezier2U();
    const float s = cvar_faceCurveCurvature;
    // compute endpoints and transform both control and endpoints to local space.
    for(unsigned i = 0; i < face.sz; ++i) {
        const M::Vector3& c0 = points[i];
        const M::Vector3& c1 = points[(i + 1) % face.sz];
        cv.curve.points.push_back(ProjXY(M::Transform(invM, (1.0f - s) * c0 + s * c1 - p0))); // must start at first endpoint!
        cv.curve.points.push_back(ProjXY(M::Transform(invM,  0.5f * c0 + 0.5f * c1 - p0)));
        cv.curve.points.push_back(ProjXY(M::Transform(invM,  s * c0 + (1.0f - s) * c1 - p0)));
        cv.curve.points.push_back(ProjXY(M::Transform(invM,  c1 - p0)));
    }
    cv.curve.points.push_back(cv.curve.points.front()); // close poly

    R::ComputeTSamples(cv.curve);
    ComputeCurveDecals(cv);
}

void ENT_Geometry::RebuildCurves() {
    for(unsigned i = 0; i < _curves.size(); ++i) {
        Curve& cv = _curves[i];
        if(true || cv.dirty) { // NOTE: always rebuild curves on Rebuild()
            RebuildCurve(cv);
            cv.dirty = false;
        }
    }
}

void ENT_Geometry::AddCurve(leda::face face, const R::Color& color) {
    Curve cv;
    cv.faceIdx = face->id();
    cv.time = 0.0f;
    cv.color = color;
    cv.dirty = true;
    _curves.push_back(cv);
}

void SetColorsFromVertexSelection(
    leda::nb::RatPolyMesh& mesh,
    const leda::node_map<bool>& selection,
    const R::Color& col0,
    const R::Color& col1)
{
    leda::node v;
    leda::edge e;

    forall_edges(e, mesh) mesh.set_color(e, col0);

    forall_nodes(v, mesh) {
        if(selection[v]) {
            mesh.set_color(v, col1);
            forall_out_edges(e, v) {
                mesh.set_source_color(e, col1);
            }
        } else mesh.set_color(v, col0);
    }
}

void SetColorsFromVertexSelection(ENT_Geometry& geom) {
    static R::Color col_unselected = R::Color::Black;
    static R::Color col_selected = R::Color::Yellow;

    leda::nb::RatPolyMesh& mesh = geom.GetRatPolyMesh();

    leda::node_map<bool> selectionMap(mesh, false);

    const std::vector<leda::node>& selectionVec = geom.GetVertexSelection();
    for(unsigned i = 0; i < selectionVec.size(); ++i) {
        selectionMap[selectionVec[i]] = true;
    }

    SetColorsFromVertexSelection(mesh, selectionMap, col_unselected, col_selected);
}

} // namespace W