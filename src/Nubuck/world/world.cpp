#include <functional>
#include <algorithm>

#include <Nubuck\common\common.h>
#include <algdriver\algdriver.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <Nubuck\math\intersections.h>
#include <renderer\mesh\plane\plane.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\cylinder\cylinder.h>
#include <renderer\nodes\r_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include <renderer\mesh\grid\grid.h>
#include <UI\outliner\outliner.h>
#include <UI\window_events.h>
#include <operators\operators.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "entity.h"
#include "world_events.h"
#include "world.h"

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

void World::Selection::ComputeCenter() {
    center = M::Vector3::Zero;
    for(unsigned i = 0; i < geomList.size(); ++i) {
        ENT_Geometry* geom = (ENT_Geometry*)geomList[i];
        center += geom->GetGlobalCenter();
    }
    center /= geomList.size();
}

void World::Selection::SignalChange() {
    world.Send(EV::def_SelectionChanged.Create(EV::Params_SelectionChanged()));
	OP::g_operators.Send(EV::def_SelectionChanged.Create(EV::Params_SelectionChanged()));
}

void World::Selection::Set(IGeometry* geom) {
    SYS::ScopedLock lock(_mtx);
	for(unsigned i = 0; i < geomList.size(); ++i) {
        ENT_Geometry* geom = (ENT_Geometry*)geomList[i];
		geom->Deselect();
	}
    geomList.clear();
    ENT_Geometry* ent = (ENT_Geometry*)geom;
    ent->Select();
    geomList.push_back(geom);
    ComputeCenter();
    SignalChange();
}

void World::Selection::Add(IGeometry* geom) {
    SYS::ScopedLock lock(_mtx);
    bool sel = 0;
	for(unsigned i = 0; !sel && i < geomList.size(); ++i)
        if(geomList[i] == geom) sel = true;
    if(!sel) {
        ENT_Geometry* ent = (ENT_Geometry*)geom;
		ent->Select();
		geomList.push_back(geom);
        ComputeCenter();
        SignalChange();
	}
}

void World::Selection::Clear() {
    SYS::ScopedLock lock(_mtx);
	for(unsigned i = 0; i < geomList.size(); ++i) {
        ENT_Geometry* geom = (ENT_Geometry*)geomList[i];
		geom->Deselect();
	}
    geomList.clear();
    SignalChange();
}

M::Vector3 World::Selection::GetGlobalCenter() { 
    SYS::ScopedLock lock(_mtx);
    ComputeCenter();
    return center; 
}

std::vector<IGeometry*> World::Selection::GetList() const { 
    SYS::ScopedLock lock(_mtx);
    return geomList; 
}

void World::Selection::SelectVertex(SelectMode mode, IGeometry* geom, leda::node vert) {
    SYS::ScopedLock lock(_mtx);
    W::ENT_Geometry* ent = (W::ENT_Geometry*)geom;
    if(SELECT_NEW == mode) ent->ClearVertexSelection();
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

void World::SetupLights(R::RenderList& renderList) {
    renderList.dirLights[0].direction      = M::Vector3(-1.0f,  1.0f,  0.0f);
    renderList.dirLights[0].diffuseColor   = R::Color(0.6f, 0.85f, 0.91f);

    renderList.dirLights[1].direction      = M::Vector3( 1.0f,  1.0f,  0.0f);
    renderList.dirLights[1].diffuseColor   = R::Color(1.0f, 0.5f, 0.15f);

    renderList.dirLights[2].direction      = M::Vector3( 0.0f, -0.5f, -1.5f);
    renderList.dirLights[2].diffuseColor   = R::Color(1.0f, 1.0f, 1.0f);
}

static float mouseX, mouseY;
static float screenWidth, screenHeight;
static float aspect;

void World::Event_Mouse(const EV::Event& event) {
    const EV::Params_Mouse& args = EV::def_Mouse.GetArgs(event);

    if(EV::Params_Mouse::MOUSE_DOWN == args.type) {
        if(EV::Params_Mouse::BUTTON_LEFT == args.button) {
            if(EV::Params_Mouse::MODIFIER_SHIFT == args.mods)
                _camArcball.StartZooming(args.x, args.y);
            else _camArcball.StartDragging(args.x, args.y);
        }
		if(EV::Params_Mouse::BUTTON_MIDDLE == args.button) {
            _camArcball.StartPanning(args.x, args.y);
        }
    }

    if(EV::Params_Mouse::MOUSE_UP == args.type) {
        if(EV::Params_Mouse::BUTTON_LEFT  == args.button) {
            _camArcball.StopDragging();
            _camArcball.StopZooming();
        }
        if(EV::Params_Mouse::BUTTON_MIDDLE == args.button)
            _camArcball.StopPanning();
    }

    bool cameraChanged = false;

    if(EV::Params_Mouse::MOUSE_MOVE == args.type) {
        _camArcball.Drag(args.x, args.y);
        _camArcball.Pan(args.x, args.y);
        _camArcball.Zoom(args.x, args.y);
        mouseX = args.x;
        mouseY = args.y;
        cameraChanged = true;
    }

    if(EV::Params_Mouse::MOUSE_WHEEL == args.type) {
        if(args.delta > 0) _camArcball.ZoomIn();
        if(args.delta < 0) _camArcball.ZoomOut();
        cameraChanged = true;
    }

    // TODO: do this once per world.Update()
    if(cameraChanged) {
        OP::g_operators.OnCameraChanged();
    }
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

void World::Event_LinkEntity(const EV::Event& event) {
    const EV::Params_LinkEntity& args = EV::def_LinkEntity.GetArgs(event);
    GEN::Pointer<Entity> entity(args.entity);
	if(W::EntityType::ENT_GEOMETRY == entity->GetType()) {
        ENT_Geometry& geom = (ENT_Geometry&)*entity;
	}
    _entities.push_back(entity);
}

void World::Event_DestroyEntity(const EV::Event& event) {
    const EV::Params_DestroyEntity& args = EV::def_DestroyEntity.GetArgs(event);
    for(unsigned i = 0; i < _entities.size(); ++i) {
        if(_entities[i]->GetID() == args.entId) {
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

void World::Event_RebuildAll(const EV::Event&) {
    RebuildAll();
}

void World::Event_Resize(const EV::Event& event) {
    const EV::Params_Resize& args = EV::def_Resize.GetArgs(event);
    _camArcball.SetScreenSize(args.width, args.height);
    aspect = (float)args.width / args.height;
    screenWidth = args.width;
    screenHeight = args.height;
}

void World::Event_Key(const EV::Event& event) {
    const EV::Params_Key& args = EV::def_Key.GetArgs(event);

    if('R' == args.keyCode) _camArcball.ResetRotation();
    if('E' == args.keyCode) _camArcball.Reset();

    if(EV::Params_Key::KEY_DOWN == args.type && !args.autoRepeat) {
        GEN::Pointer<IPhase> phase = ALG::gs_algorithm.GetPhase();
        if(phase.IsValid()) phase->OnKeyPressed((char)args.keyCode);
    }
}

void World::Grid_Build() {
    R::Grid grid(4, 20.0f);
    _gridMesh = R::meshMgr.Create(grid.GetDesc());
    _gridTFMesh = R::meshMgr.Create(_gridMesh);
    R::meshMgr.GetMesh(_gridTFMesh).SetTransform(M::Mat4::Identity());
}

R::MeshJob World::Grid_GetRenderJob() {
    R::MeshJob meshJob;
    meshJob.fx = "Unlit";
    meshJob.layer = 0;
    meshJob.material = R::Material::White;
    meshJob.tfmesh = _gridTFMesh;
    meshJob.primType = 0;
    return meshJob;
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

World::BoundingBox::BoundingBox(const ENT_Geometry* geom) : geom(geom) {
    WireframeBox meshDesc(M::Scale(geom->GetBoundingBox(), 1.2f));
    mesh = R::meshMgr.Create(meshDesc.GetDesc());
    tfmesh = R::meshMgr.Create(mesh);
    Transform();
}

void World::BoundingBox::Transform() {
    R::meshMgr.GetMesh(tfmesh).SetTransform(geom->GetTransformationMatrix());
}

void World::BBoxes_BuildFromSelection() {
    _bboxes.clear();
    std::vector<IGeometry*> geomList = _selection.GetList();
    for(unsigned i = 0; i < geomList.size(); ++i) {
        _bboxes.push_back(GEN::MakePtr(new BoundingBox((const ENT_Geometry*)geomList[i])));
    }
}

void World::BBoxes_GetRenderJobs(std::vector<R::MeshJob>& rjobs) {
    R::MeshJob rjob;
    rjob.fx         = "Unlit";
    rjob.layer      = 0;
    rjob.material   = R::Material::White;
    rjob.primType   = 0;
    for(unsigned i = 0; i < _bboxes.size(); ++i) {
        _bboxes[i]->Transform();
        rjob.tfmesh = _bboxes[i]->tfmesh;
        rjobs.push_back(rjob);
    }
}

World::World(void) : _camArcball(800, 400) /* init values arbitrary */
{
    AddEventHandler(EV::def_Apocalypse,           this, &World::Event_Apocalypse);
    AddEventHandler(EV::def_LinkEntity,           this, &World::Event_LinkEntity);
    AddEventHandler(EV::def_DestroyEntity,        this, &World::Event_DestroyEntity);
    AddEventHandler(EV::def_SelectionChanged,     this, &World::Event_SelectionChanged);
    AddEventHandler(EV::def_RebuildAll,           this, &World::Event_RebuildAll);
    AddEventHandler(EV::def_Resize,               this, &World::Event_Resize);
    AddEventHandler(EV::def_Mouse,                this, &World::Event_Mouse);
    AddEventHandler(EV::def_Key,                  this, &World::Event_Key);

    Grid_Build();

    // set default camera position
    M::TransformTRS tf;
    tf.trans = M::Vector3(0.0f, 0.0f, -28.0f);
    tf.rot = M::Quaternion(0.876f, M::Vector3(-0.298f, 0.355f, 0.126f));
    tf.scale = 1.0f;
    _camArcball.SetTransform(tf);
}

M::Ray World::PickingRay(const M::Vector2& mouseCoords) {
    M::Matrix4 projectionMat = M::Mat4::Perspective(45.0f, aspect, 0.1f, 1000.0f);
    M::Matrix4 invWorldMat;
    M::TryInvert(_camArcball.GetWorldMatrix(), invWorldMat);
    M::Ray ray;
    ray.origin = M::Transform(invWorldMat, M::Vector3::Zero);
    ray.direction = UnprojectPoint(projectionMat, _camArcball.GetWorldMatrix(), screenWidth, screenHeight, mouseCoords);
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

bool World::Trace(const M::Ray& ray, ENT_Geometry** ret) {
    SYS::ScopedLock lockEntities(_entitiesMtx);
    for(unsigned i = 0; i < _entities.size(); ++i) {
        GEN::Pointer<Entity> entity = _entities[i];
        if(EntityType::ENT_GEOMETRY == entity->GetType()) {
            ENT_Geometry& geom = static_cast<ENT_Geometry&>(*entity);
            M::Box bbox = geom.GetBoundingBox();
            SetCenterPosition(bbox, M::Transform(geom.GetTransformationMatrix(), GetCenterPosition(bbox)));
            if(geom.IsSolid() && M::IS::Intersects(ray, bbox)) {
                *ret = &geom;
                return true;
            }
        }
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

    BBoxes_BuildFromSelection();

    for(unsigned i = 0; i < _entities.size(); ++i) {
        GEN::Pointer<Entity> entity = _entities[i];
		if(entity->IsDead() && !entity->IsSelected()) {
			_entities[i]->OnDestroy();
            std::swap(_entities[i], _entities.back());
            _entities.erase(_entities.end() - 1);
		}
        else if(EntityType::ENT_GEOMETRY == entity->GetType()) {
            ENT_Geometry& geom = static_cast<ENT_Geometry&>(*entity);
			geom.HandleEvents();
            geom.CompileMesh();
            geom.FrameUpdate();
            geom.BuildRenderList();
        }
    }
}

void World::Render(R::RenderList& renderList) {
    SetupLights(renderList);

    renderList.worldMat = _camArcball.GetWorldMatrix();
    renderList.meshJobs.clear();

    renderList.meshJobs.push_back(Grid_GetRenderJob());

    BBoxes_GetRenderJobs(renderList.meshJobs);

    SYS::ScopedLock lockEntities(_entitiesMtx);

    for(unsigned i = 0; i < _entities.size(); ++i) {
        if(EntityType::ENT_GEOMETRY == _entities[i]->GetType()) {
            ENT_Geometry& geom = static_cast<ENT_Geometry&>(*_entities[i]);
            renderList.meshJobs.insert(renderList.meshJobs.end(),
                geom.GetRenderList().meshJobs.begin(),
                geom.GetRenderList().meshJobs.end());
        }
    }
}

ISelection* World::GetSelection() {
    return &_selection;
}

IGeometry* World::CreateGeometry() {
    entIdCntMtx.Lock();
    unsigned entId = entIdCnt++;
    entIdCntMtx.Unlock();

    ENT_Geometry* geom = new ENT_Geometry;
    geom->SetType(EntityType::ENT_GEOMETRY);
    geom->SetID(entId);
    geom->SetFxName("LitDirectional");

    EntTransform transform;
    transform.position = M::Vector3::Zero;
    transform.scale = M::Vector3(1.0f, 1.0f, 1.0f);
    transform.rotation = M::Mat3::Identity();
    geom->SetTransform(transform);

    EV::Params_LinkEntity args;
    args.entity = geom;
    Send(EV::def_LinkEntity.Create(args));

    return geom;
}

DWORD World::Thread_Func(void) {
    while(true) {
        Update();
        // Sleep(10);
    }
}

} // namespace W
