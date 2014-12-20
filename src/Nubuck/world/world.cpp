#include <functional>
#include <algorithm>

#include <Nubuck\common\common.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <Nubuck\math\intersections.h>
#include <Nubuck\animation\animation.h>
#include <Nubuck\animation\animator.h>
#include <renderer\mesh\plane\plane.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\cylinder\cylinder.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include <renderer\mesh\grid\grid.h>
#include <UI\outliner\outliner.h>
#include <UI\window_events.h>
#include <UI\userinterface.h>
#include <operators\operators.h>
#include <operators\operator_driver.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <world\entities\ent_text\ent_text.h>
#include <world\entities\ent_transform_gizmo\ent_transform_gizmo.h>
#include <nubuck_private.h>
#include "entity.h"
#include "world_events.h"
#include "world.h"

COM::Config::Variable<int> cvar_w_showGrid("w_showGrid", 1);
COM::Config::Variable<int> cvar_w_gridPlane("w_gridPlane", 0); // 0 = xy, 1 = yz, 2 = xz

namespace {

struct WireframeBox {
private:
    std::vector<R::Mesh::Vertex>    _vertices;
    std::vector<R::Mesh::Index>     _indices;
public:
    explicit WireframeBox(const M::Box& box) {
        M::Vector3 pos[] = {
            M::Vector3(box.min.x, box.min.y, box.max.z),
            M::Vector3(box.max.x, box.min.y, box.max.z),
            M::Vector3(box.max.x, box.max.y, box.max.z),
            M::Vector3(box.min.x, box.max.y, box.max.z),
            M::Vector3(box.min.x, box.min.y, box.min.z),
            M::Vector3(box.max.x, box.min.y, box.min.z),
            M::Vector3(box.max.x, box.max.y, box.min.z),
            M::Vector3(box.min.x, box.max.y, box.min.z),
        };
        unsigned numVertices = 8;
        R::Mesh::Index indices[] = {
            0, 1, 1, 2, 2, 3, 3, 0, // front
            4, 5, 5, 6, 6, 7, 7, 4, // back
            3, 7, 2, 6,             // top
            0, 4, 1, 5              // bottom
        };
        unsigned numIndices = sizeof(indices) / sizeof(R::Mesh::Index);
        R::Mesh::Vertex vert;
        vert.color = R::Color::White;
        for(unsigned i = 0; i < 8; ++i) {
            vert.position = pos[i];
            _vertices.push_back(vert);
        }
        for(unsigned i = 0; i < numIndices; ++i)
            _indices.push_back(indices[i]);
    }

    R::Mesh::Desc GetDesc() {
        R::Mesh::Desc desc;
        desc.vertices       = &_vertices[0];
        desc.numVertices    = _vertices.size();
        desc.indices        = &_indices[0];
        desc.numIndices     = _indices.size();
        desc.primType       = GL_LINES;
        return desc;
    }
};

} // unnamed namespace

namespace W {

static SYS::SpinLock entIdCntMtx;
static unsigned entIdCnt = 0;

SYS::Semaphore g_worldSem(0);
static int rlIdx = 1;

World world;

// Selection impl ---

void World::Selection::_Clear() {
    Entity *next, *ent = head;
    while(ent) {
        ent->Deselect();
        next = ent->selectionLink.next;
        ent->selectionLink.SetNull();
        ent = next;
    }
    head = NULL;
}

void World::Selection::_Select(Entity* ent) {
    ent->selectionLink.prev = NULL;
    ent->selectionLink.next = head;
    if(head) head->selectionLink.prev = ent;
    head = ent;

    ent->Select();
}

void World::Selection::ComputeCenter() {
    int size = 0;
    center = M::Vector3::Zero;
    const Entity* ent = head;
    while(ent) {
        center += ent->GetGlobalCenter();
        ent = ent->selectionLink.next;
        size++;
    }
    center /= size;
}

void World::Selection::SignalChange() {
    world.Send(ev_w_selectionChanged.Tag());
	OP::g_operators.Send(ev_w_selectionChanged.Tag());
    g_ui.GetOutliner().Send(ev_w_selectionChanged.Tag());
}

void World::Selection::Set(Entity* ent) {
    SYS::ScopedLock lock(_mtx);
    _Clear();
    _Select(ent);
    ComputeCenter();
    SignalChange();
}

void World::Selection::Add(Entity* ent) {
    if(ent->IsSelected()) return;

    SYS::ScopedLock lock(_mtx);
    _Select(ent);
    ComputeCenter();
    SignalChange();
}

void World::Selection::Clear() {
    SYS::ScopedLock lock(_mtx);
    _Clear();
    ComputeCenter(); // essentially sets center to 0
    SignalChange();
}

M::Vector3 World::Selection::GetGlobalCenter() {
    SYS::ScopedLock lock(_mtx);
    ComputeCenter();
    return center;
}

template<typename TYPE>
void GetFilteredList(const EntityType::Enum type, const Entity* head, std::vector<TYPE*>& filtered) {
    filtered.clear();
    const Entity* ent = head;
    while(ent) {
        if(type == ent->GetType()) {
            filtered.push_back((TYPE*)ent); // ugly but safe downcast
        }
        ent = ent->selectionLink.next;
    }
}

std::vector<ENT_Geometry*> World::Selection::GetGeometryList() const {
    SYS::ScopedLock lock(_mtx);
    std::vector<ENT_Geometry*> filtered;
    GetFilteredList(EntityType::ENT_GEOMETRY, head, filtered);
    return filtered;
}

std::vector<ENT_Text*> World::Selection::GetTextList() const {
    SYS::ScopedLock lock(_mtx);
    std::vector<ENT_Text*> filtered;
    GetFilteredList(EntityType::ENT_TEXT, head, filtered);
    return filtered;
}

void World::Selection::SelectVertex_New(ENT_Geometry* geom, leda::node vert) {
    SYS::ScopedLock lock(_mtx);
    W::ENT_Geometry* ent = (W::ENT_Geometry*)geom;
    ent->ClearVertexSelection();
    ent->Select(vert);
    SignalChange();
}

void World::Selection::SelectVertex_Add(ENT_Geometry* geom, leda::node vert) {
    SYS::ScopedLock lock(_mtx);
    W::ENT_Geometry* ent = (W::ENT_Geometry*)geom;
    ent->Select(vert);
    SignalChange();
}

// --- Selection Impl

static M::Vector3 Transform(const M::Matrix4& mat, const M::Vector3& vec, float w) {
    M::Vector3 ret;
    for(int i = 0; i < 3; ++i) {
        float el = 0.0f;
        for(int j = 0; j < 3; ++j) {
            el += mat.mat[i + 4 * j] * vec.vec[j];
        }
        ret.vec[i] = el + w * mat.mat[i + 12];
    }
    return ret;
}

// given a point p in window space, return a point q in world space
// that lies on the near clipping plane with the same window space
// coordinates as p
static M::Vector3 UnprojectPoint(
    const M::Matrix4& projectionMat,
    const M::Matrix4& worldMat,
    float screenWidth,
    float screenHeight,
    const M::Vector2& p)
{
    M::Vector3 q;

    // in NDC
    q.x = 2.0f * (p.x / screenWidth) - 1.0f;
    q.y = -(2.0f * (p.y / screenHeight) - 1.0f);
    q.z = -1.0f; // on near clipping plane

    // clip.w = -eye.z, where eye.z is z-coord
    // of near clipping plane
    const float fov = M::Deg2Rad(45.0f);
    float clipW = 1.0f / tan(0.5f * fov);

    // in clip space
    q *= clipW; // q.w = clipW

    M::Matrix4 M;
    M::TryInvert(projectionMat, M);
    M::Vector3 t = Transform(M, q, clipW); // in world space
    // printf("eye = %f, %f, %f\n", t.x, t.y, t.z);
    M::Matrix3 normalMat = M::Inverse(M::RotationOf(worldMat));
    return Transform(normalMat, t);
}

void World::SetDefaultLights() {
    const R::Color gray = R::Color(0.5f, 0.5f, 0.5f);

    _dirLights[0].direction      = M::Vector3(-1.0f,  1.0f,  0.0f);
    _dirLights[0].diffuseColor   = gray;

    _dirLights[1].direction      = M::Vector3( 1.0f,  1.0f,  0.0f);
    _dirLights[1].diffuseColor   = gray;

    _dirLights[2].direction      = M::Vector3( 0.0f, -0.5f, -1.5f);
    _dirLights[2].diffuseColor   = R::Color::White;
}

void World::SetupLights(R::RenderList& renderList) {
    for(int i = 0; i < 3; ++i) {
        renderList.dirLights[i].direction = _dirLights[i].direction;
        renderList.dirLights[i].diffuseColor = _dirLights[i].diffuseColor;
    }
}

static float mouseX, mouseY;
static float screenWidth, screenHeight;
static float aspect;

void World::Event_Mouse(const EV::MouseEvent& event) {
    HandleMouseEvent(event);
}

GEN::Pointer<Entity> World::FindByEntityID(unsigned entId) {
    for(unsigned i = 0; i < _entities.size(); ++i) {
        if(_entities[i]->GetID() == entId) return _entities[i];
    }
    return GEN::Pointer<Entity>();
}

void World::Event_Apocalypse(const EV::Event& event) {
    _entities.clear();
}

void World::Event_LinkEntity(const EV::Arg<Entity*>& event) {
    GEN::Pointer<Entity> entity(event.value);
	if(W::EntityType::ENT_GEOMETRY == entity->GetType()) {
        ENT_Geometry& geom = (ENT_Geometry&)*entity;
	}
    _entities.push_back(entity);
}

void World::Event_DestroyEntity(const EV::Arg<unsigned>& event) {
    unsigned entId = event.value;
    for(unsigned i = 0; i < _entities.size(); ++i) {
        if(_entities[i]->GetID() == entId) {
			_entities[i]->OnDestroy();
            std::swap(_entities[i], _entities.back());
            _entities.erase(_entities.end() - 1);
        }
    }
	event.Accept();
}

void World::Event_SelectionChanged(const EV::Event&) {
    // bboxes get updated in Update()
}

void World::Event_RebuildAll(const EV::Event& event) {
    RebuildAll();
    event.Accept();
}

void World::Event_EditModeChanged(const EV::Arg<int>& event) {
    int editMode = event.value;
    std::vector<ENT_Geometry*> geomSel = _selection.GetGeometryList();
    if(!geomSel.empty()) {
        ENT_Geometry* ent = (ENT_Geometry*)geomSel.front();
        ent->SetEditMode(editMode_t::Enum(editMode));
    }
}

void World::Event_Resize(const EV::ResizeEvent& event) {
    _camArcball.SetScreenSize(event.width, event.height);
    aspect = (float)event.width / event.height;
    screenWidth = event.width;
    screenHeight = event.height;
}

void World::Event_Key(const EV::KeyEvent& event) {
    // numpad scancodes of generic usb keyboard
    static const int numpad[] = {
        82,
        79, 80, 81,
        75, 76, 77,
        71, 72, 73
    };

    const float transitionDur = 0.25f;

    if('R' == event.keyCode) {
        _camArcball.ResetRotation();
    }
    if('E' == event.keyCode) {
        _camArcball.Reset();
    }
    if(numpad[1] == event.nativeScanCode) {
        if(EV::KeyEvent::MODIFIER_SHIFT & event.mods) {
            _camArcball.RotateTo(M::Quat::RotateAxis(M::Vector3(0.0f, 1.0f, 0.0f), 180.0f), transitionDur); // back view
        } else {
            _camArcball.RotateTo(M::Quat::Identity(), transitionDur); // front view
        }
    }
    if(numpad[3] == event.nativeScanCode) {
        if(EV::KeyEvent::MODIFIER_SHIFT & event.mods) {
            _camArcball.RotateTo(M::Quat::RotateAxis(M::Vector3(0.0f, 1.0f, 0.0f), -90.0f), transitionDur); // left view
        } else {
            _camArcball.RotateTo(M::Quat::RotateAxis(M::Vector3(0.0f, 1.0f, 0.0f), 90.0f), transitionDur); // right view
        }
    }
    if(numpad[7] == event.nativeScanCode) {
        if(EV::KeyEvent::MODIFIER_SHIFT & event.mods) {
            _camArcball.RotateTo(M::Quat::RotateAxis(M::Vector3(1.0f, 0.0f, 0.0f), 90.0f), transitionDur); // bottom view
        } else {
            _camArcball.RotateTo(M::Quat::RotateAxis(M::Vector3(1.0f, 0.0f, 0.0f), -90.0f), transitionDur); // top view
        }
    }
    if(numpad[5] == event.nativeScanCode) _camArcball.ToggleProjection(transitionDur);
}

void World::Grid_Build() {
    R::Grid grid(4, 20.0f);
    _gridMesh = R::meshMgr.Create(grid.GetDesc());
    _gridTFMesh = R::meshMgr.Create(_gridMesh);
    R::meshMgr.GetMesh(_gridTFMesh).SetTransform(M::Mat4::Identity());
}

void World::Grid_GetRenderJobs(std::vector<R::MeshJob>& rjobs) {
    R::MeshJob meshJob;
    meshJob.fx = "Unlit";
    meshJob.material = R::Material::White;
    meshJob.tfmesh = _gridTFMesh;
    meshJob.primType = 0;

    M::Matrix4 gridTransform;
    switch(cvar_w_gridPlane) {
    case 0:
        gridTransform = M::Mat4::Identity();
        break;
    case 1:
        gridTransform = M::Mat4::RotateX(90.0f);
        break;
    case 2:
        gridTransform = M::Mat4::RotateZ(90.0f);
        break;
    default:
        COM_assert(0 && "invalid grid plane");
    };
    R::meshMgr.GetMesh(_gridTFMesh).SetTransform(gridTransform);

    meshJob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
    rjobs.push_back(meshJob);

    meshJob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
    rjobs.push_back(meshJob);
}

void World::BoundingBox::Destroy() {
    if(mesh) {
        R::meshMgr.Destroy(mesh);
        mesh = NULL;
    }
    if(tfmesh) {
        R::meshMgr.Destroy(tfmesh);
        tfmesh = NULL;
    }
}

World::BoundingBox::BoundingBox(const Entity* entity) : entity(entity) {
    const M::Vector3 vf = 0.5f * M::Vector3(1.0f, 1.0f, 1.0f);
    M::Box bbox = entity->GetBoundingBox();
    bbox.min -= vf;
    bbox.max += vf;
    WireframeBox meshDesc(bbox);
    mesh = R::meshMgr.Create(meshDesc.GetDesc());
    tfmesh = R::meshMgr.Create(mesh);
    Transform();
}

void World::BoundingBox::Transform() {
    R::meshMgr.GetMesh(tfmesh).SetTransform(entity->GetObjectToWorldMatrix());
}

void World::BBoxes_BuildFromSelection() {
    _bboxes.clear();
    Entity* ent = _selection.Head();
    while(ent) {
        _bboxes.push_back(GEN::MakePtr(new BoundingBox(ent)));
        ent = ent->selectionLink.next;
    }
}

void World::BBoxes_GetRenderJobs(std::vector<R::MeshJob>& rjobs) {
    R::MeshJob rjob;
    rjob.fx         = "Unlit";
    rjob.material   = R::Material::White;
    rjob.primType   = 0;
    for(unsigned i = 0; i < _bboxes.size(); ++i) {
        _bboxes[i]->Transform();
        rjob.tfmesh = _bboxes[i]->tfmesh;

        rjob.layer  = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
        rjobs.push_back(rjob);

        rjob.layer  = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
        rjobs.push_back(rjob);
    }
}

World::World(void) : _camArcball(800, 400) /* init values arbitrary */
{
    SetDefaultLights();
}

void World::Init() {
    AddEventHandler(ev_w_apocalypse,           this, &World::Event_Apocalypse);
    AddEventHandler(ev_w_linkEntity,           this, &World::Event_LinkEntity);
    AddEventHandler(ev_w_destroyEntity,        this, &World::Event_DestroyEntity);
    AddEventHandler(ev_w_selectionChanged,     this, &World::Event_SelectionChanged);
    AddEventHandler(ev_w_rebuildAll,           this, &World::Event_RebuildAll);
    AddEventHandler(ev_w_editModeChanged,      this, &World::Event_EditModeChanged);
    AddEventHandler(ev_resize,               this, &World::Event_Resize);
    AddEventHandler(ev_mouse,                this, &World::Event_Mouse);
    AddEventHandler(ev_key,                  this, &World::Event_Key);

    Grid_Build();

    _globalTransformGizmo = CreateTransformGizmo();
}

const R::DirectionalLight& World::GetDirectionalLight(const int idx) const {
    COM_assert(0 <= idx && idx < 3);
    return _dirLights[idx];
}

void World::SetDirectionalLight(const int idx, const R::DirectionalLight& dirLight) {
    COM_assert(0 <= idx && idx < 3);
    _dirLights[idx] = dirLight;
}

M::Ray World::PickingRay(const M::Vector2& mouseCoords) {
    M::Matrix4 projectionMat = M::Mat4::Perspective(45.0f, aspect, 0.1f, 1000.0f);
    M::Matrix4 invWorldMat;
    M::TryInvert(_camArcball.GetWorldToEyeMatrix(), invWorldMat);
    M::Ray ray;
    ray.origin = M::Transform(invWorldMat, M::Vector3::Zero);
    ray.direction = UnprojectPoint(projectionMat, _camArcball.GetWorldToEyeMatrix(), screenWidth, screenHeight, mouseCoords);
    ray.direction.Normalize();
    return ray;
}

static void SetCenterPosition(M::Box& box, const M::Vector3& center) {
    const M::Vector3 oldCenter = 0.5f * (box.max - box.min) + box.min;
    const M::Vector3 d = (center - oldCenter);
    box.min += d;
    box.max += d;
}

static M::Vector3 GetCenterPosition(const M::Box& box) {
    return 0.5f * (box.max - box.min) + box.min;
}

bool World::Trace(const M::Ray& ray, Entity** ret, entityFilter_t filter) {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    for(unsigned i = 0; i < _entities.size(); ++i) {
        GEN::Pointer<Entity> entity = _entities[i];
        if(!entity->IsDead() && filter(*entity)) {
            M::Box bbox = entity->GetBoundingBox();
            SetCenterPosition(bbox, M::Transform(entity->GetObjectToWorldMatrix(), GetCenterPosition(bbox)));
            if(entity->IsSolid() && M::IS::Intersects(ray, bbox)) {
                *ret = entity.Raw();
                return true;
            }
        }
    }
    return false;
}

static bool Filter_AcceptAll(const Entity&) { return true; }
static bool Filter_IsGeometry(const Entity& ent) { return EntityType::ENT_GEOMETRY == ent.GetType(); }

bool World::TraceEntity(const M::Ray& ray, Entity** ret) {
    return Trace(ray, ret, Filter_AcceptAll);
}

bool World::TraceGeometry(const M::Ray& ray, ENT_Geometry** ret) {
    Entity* hit = NULL;
    if(Trace(ray, &hit, Filter_IsGeometry)) {
        *ret = static_cast<ENT_Geometry*>(hit); // ugly but safe downcast
        return true;
    }
    return false;
}

void World::RebuildAll() {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    for(unsigned i = 0; i < _entities.size(); ++i) {
        GEN::Pointer<Entity>& entity = _entities[i];
        if(EntityType::ENT_GEOMETRY == entity->GetType()) {
            ENT_Geometry& geom = static_cast<ENT_Geometry&>(*entity);
            geom.Rebuild();
        }
    }
}

void World::Update(void) {
    _secsPassed = _timer.Stop();
    _timePassed += _secsPassed;
    _timer.Start();

    SYS::ScopedLock lockEntities(_entitiesMtx);

    HandleEvents();

    unsigned entIdx = 0;
    while(entIdx < _entities.size()) {
        GEN::Pointer<Entity> entity = _entities[entIdx];
		if(entity->IsDead() && !entity->IsSelected()) {
            _entities[entIdx]->OnDestroy();
            std::swap(_entities[entIdx], _entities.back());
            _entities.pop_back();
        } else entIdx++;
    }

    // it's important to build the bboxes after dead entities have been destroyed,
    // so that bboxes don't have dangling pointers
    BBoxes_BuildFromSelection();

    for(unsigned i = 0; i < _entities.size(); ++i) {
        GEN::Pointer<Entity> entity = _entities[i];
        if(EntityType::ENT_GEOMETRY == entity->GetType()) {
            ENT_Geometry& geom = static_cast<ENT_Geometry&>(*entity);
			geom.HandleEvents();
            geom.CompileMesh();
            geom.FrameUpdate();
            geom.BuildRenderList();
        }
    }

    _camArcball.FrameUpdate(_secsPassed);

    if(OP::g_operators.GetDriver().IsBlocked() || OP::g_operators.IsDriverIdle()) {
        A::g_animator.Move(_secsPassed);

        // RebuildAll
        for(unsigned i = 0; i < _entities.size(); ++i) {
            GEN::Pointer<Entity>& entity = _entities[i];
            if(EntityType::ENT_GEOMETRY == entity->GetType()) {
                ENT_Geometry& geom = static_cast<ENT_Geometry&>(*entity);
                geom.Rebuild();
            }
        }

        // make sure operator is signaled after entities have been rebuilt
        A::g_animator.EndFrame();
    }
}

void World::Render(R::RenderList& renderList) {
    SetupLights(renderList);

    renderList.projWeight = _camArcball.GetProjectionWeight();
    renderList.zoom = _camArcball.GetZoom();
    renderList.worldMat = _camArcball.GetWorldToEyeMatrix();

    if(g_showRenderViewControls) {
        BBoxes_GetRenderJobs(renderList.meshJobs);
    }
    if(cvar_w_showGrid) Grid_GetRenderJobs(renderList.meshJobs);

    SYS::ScopedLock lockEntities(_entitiesMtx);

    for(unsigned i = 0; i < _entities.size(); ++i) {
        if(!_entities[i]->IsDead()) {
            if(EntityType::ENT_GEOMETRY == _entities[i]->GetType()) {
                ENT_Geometry& geom = static_cast<ENT_Geometry&>(*_entities[i]);
                renderList.meshJobs.insert(renderList.meshJobs.end(),
                    geom.GetRenderList().meshJobs.begin(),
                    geom.GetRenderList().meshJobs.end());
            } else if(EntityType::ENT_TEXT == _entities[i]->GetType()) {
                ENT_Text& text = static_cast<ENT_Text&>(*_entities[i]);
                text.GetRenderJobs(renderList);
            } else if(EntityType::ENT_TRANSFORM_GIZMO == _entities[i]->GetType()) {
                ENT_TransformGizmo& transformGizmo = static_cast<ENT_TransformGizmo&>(*_entities[i]);
                transformGizmo.GetRenderJobs(renderList);
            }
        }
    }
}

void World::HandleMouseEvent(const EV::MouseEvent& event) {
    if(EV::MouseEvent::MOUSE_DOWN == event.type) {
        if(EV::MouseEvent::BUTTON_LEFT == event.button) {
            if(EV::MouseEvent::MODIFIER_SHIFT == event.mods)
                _camArcball.StartZooming(event.x, event.y);
            else if(EV::MouseEvent::MODIFIER_CTRL == event.mods)
                _camArcball.StartPanning(event.x, event.y);
            else _camArcball.StartDragging(event.x, event.y);
        }
		if(EV::MouseEvent::BUTTON_MIDDLE == event.button) {
            _camArcball.StartPanning(event.x, event.y);
        }
    }

    if(EV::MouseEvent::MOUSE_UP == event.type) {
        if(EV::MouseEvent::BUTTON_LEFT  == event.button) {
            _camArcball.StopDragging();
            _camArcball.StopPanning();
            _camArcball.StopZooming();
        }
        if(EV::MouseEvent::BUTTON_MIDDLE == event.button)
            _camArcball.StopPanning();
    }

    if(EV::MouseEvent::MOUSE_MOVE == event.type) {
        _camArcball.Drag(event.x, event.y);
        _camArcball.Pan(event.x, event.y);
        _camArcball.Zoom(event.x, event.y);
        mouseX = event.x;
        mouseY = event.y;
    }

    if(EV::MouseEvent::MOUSE_WHEEL == event.type) {
        if(event.delta > 0) _camArcball.ZoomIn();
        if(event.delta < 0) _camArcball.ZoomOut();
    }
}

ENT_Geometry* World::CreateGeometry() {
    entIdCntMtx.Lock();
    unsigned entId = entIdCnt++;
    entIdCntMtx.Unlock();

    ENT_Geometry* geom = new ENT_Geometry;
    geom->SetType(EntityType::ENT_GEOMETRY);
    geom->SetID(entId);
    geom->SetFxName("LitDirectional");

    geom->SetPosition(M::Vector3::Zero);
    geom->SetOrientation(M::Quat::Identity());
    geom->SetScale(M::Vector3(1.0f, 1.0f, 1.0f));

    Send(ev_w_linkEntity.Tag(geom));

    return geom;
}

ENT_Text* World::CreateText() {
    entIdCntMtx.Lock();
    unsigned entId = entIdCnt++;
    entIdCntMtx.Unlock();

    ENT_Text* text = new ENT_Text();
    text->SetID(entId);

    text->SetPosition(M::Vector3::Zero);
    text->SetOrientation(M::Quat::Identity());
    text->SetScale(M::Vector3(1.0f, 1.0f, 1.0f));

    Send(ev_w_linkEntity.Tag(text));

    return text;
}

ENT_TransformGizmo* World::CreateTransformGizmo() {
    entIdCntMtx.Lock();
    unsigned entId = entIdCnt++;
    entIdCntMtx.Unlock();

    ENT_TransformGizmo* transformGizmo = new ENT_TransformGizmo();
    transformGizmo->SetID(entId);

    transformGizmo->SetPosition(M::Vector3::Zero);
    transformGizmo->SetOrientation(M::Quat::Identity());
    transformGizmo->SetScale(M::Vector3(1.0f, 1.0f, 1.0f));

    Send(ev_w_linkEntity.Tag(transformGizmo));

    return transformGizmo;
}

ENT_TransformGizmo* World::GlobalTransformGizmo() {
    return _globalTransformGizmo;
}

void World::ClearSelection() {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    _selection.Clear();
}

void World::Select_New(Entity* ent) {
    _selection.Set(ent);
}

void World::Select_Add(Entity* ent) {
    _selection.Add(ent);
}

void World::SelectVertex_New(ENT_Geometry* geom, const leda::node vert) {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    _selection.SelectVertex_New(geom, vert);
}

void World::SelectVertex_Add(ENT_Geometry* geom, const leda::node vert) {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    _selection.SelectVertex_Add(geom, vert);
}

std::vector<ENT_Geometry*> World::SelectedGeometry() {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    return _selection.GetGeometryList();
}

M::Vector3 World::GlobalCenterOfSelection() {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    return _selection.GetGlobalCenter();
}

Entity* World::FirstSelectedEntity() {
    return _selection.Head();
}

Entity* World::NextSelectedEntity(Entity* ent) {
    return ent->selectionLink.next;
}

DWORD World::Thread_Func(void) {
    while(true) {
        Update();
        // Sleep(10);
    }
}

} // namespace W
