#pragma once

#include <limits>
#include <map>
#include <vector>
#include <queue>

#include <Nubuck\generic\pointer.h>
#include <system\timer\timer.h>
#include <system\thread\thread.h>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\semaphore.h>
#include <common\types.h>
#include <Nubuck\math\vector2.h>
#include <Nubuck\math\ray.h>
#include <renderer\renderer.h>
#include <camera\arcball_camera.h>
#include <Nubuck\events\events.h>
#include <Nubuck\events\core_events.h>
#include <world\editmode\editmode.h>
#include "entity.h"

namespace EV {

struct Params_Mouse;

} // namespace EV

namespace W {

    class ENT_Polyhedron;
    class ENT_Mesh;
    class ENT_Geometry;
    class ENT_Text;
    class ENT_TransformGizmo;

    extern SYS::Semaphore g_worldSem;

    struct EntityInf {
        unsigned    entId;
        std::string name;
        void*       inf;
    };

    class World : public SYS::Thread, public EV::EventHandler<> {
    private:
        DECL_HANDLE_EVENTS(World)

        // NOTE: selection is managed by operators ONLY
        class Selection {
        private:
            mutable SYS::SpinLock _mtx;

            Entity*     head;
            M::Vector3  center; // in world space

            void _Clear();
            void _Select(Entity* ent);

            void ComputeCenter();
            void SignalChange();
        public:
            Selection() : head(NULL) { }

            Entity* Head() { return head; }

            void Set(Entity* ent);
            void Add(Entity* ent);
            void Clear();

            M::Vector3                  GetGlobalCenter();
            std::vector<ENT_Geometry*>  GetGeometryList() const;
            std::vector<ENT_Text*>      GetTextList() const;

            void SelectVertex_New(ENT_Geometry* geom, leda::node vert);
            void SelectVertex_Add(ENT_Geometry* geom, leda::node vert);
        } _selection;

        std::vector<GEN::Pointer<Entity> > _entities;
        SYS::SpinLock _entitiesMtx;

        ENT_TransformGizmo* _globalTransformGizmo;

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        R::DirectionalLight _dirLights[3];

        void SetDefaultLights();
        void SetupLights(R::RenderList& renderList);

        ArcballCamera _camArcball;

        GEN::Pointer<Entity> FindByEntityID(unsigned entId);

        R::meshPtr_t    _gridMesh;
        R::tfmeshPtr_t  _gridTFMesh;

        void Grid_Build();
        void Grid_GetRenderJobs(std::vector<R::MeshJob>& rjobs);

        struct BoundingBox {
            const Entity*   entity;
            R::meshPtr_t    mesh;
            R::tfmeshPtr_t  tfmesh;

            void Destroy();

            BoundingBox() : entity(NULL), mesh(NULL), tfmesh(NULL) { }
            BoundingBox(const Entity* entity);
            ~BoundingBox() { Destroy(); }
            void Transform();
        };
        std::vector<GEN::Pointer<BoundingBox> > _bboxes;
        void                        BBoxes_BuildFromSelection();
        void                        BBoxes_GetRenderJobs(std::vector<R::MeshJob>& rjobs);

        EditMode _editMode;

        typedef bool (*entityFilter_t)(const Entity& ent);

        bool    Trace(const M::Ray& ray, Entity** ent, entityFilter_t filter);

#pragma region EventHandlers
        void Event_Apocalypse(const EV::Event& event);
        void Event_LinkEntity(const EV::Arg<Entity*>& event);
        void Event_DestroyEntity(const EV::Arg<unsigned>& event);
        void Event_SelectionChanged(const EV::Event& event);
        void Event_RebuildAll(const EV::Event& event);
        void Event_EditModeChanged(const EV::Arg<int>& event);
        void Event_Resize(const EV::ResizeEvent& event);
        void Event_Mouse(const EV::MouseEvent& event);
        void Event_Key(const EV::KeyEvent& event);
// region EventHandlers
#pragma endregion
    public:
		World(void);

        void Init();

        const R::DirectionalLight&  GetDirectionalLight(const int idx) const;
        void                        SetDirectionalLight(const int idx, const R::DirectionalLight& dirLight);

        const EditMode& GetEditMode() const { return _editMode; }
        EditMode&       GetEditMode() { return _editMode; };

        ArcballCamera&  GetCamera() { return _camArcball; }

        M::Matrix4 GetCameraMatrix() const { return _camArcball.GetWorldToEyeMatrix(); }
        M::Matrix4 GetModelView() const { return _camArcball.GetWorldToEyeMatrix(); }

        M::Ray  PickingRay(const M::Vector2& mouseCoords);
        bool    TraceEntity(const M::Ray& ray, Entity** ret);
        bool    TraceGeometry(const M::Ray& ray, ENT_Geometry** ret);

        void RebuildAll();
        void Update(void);
        void Render(R::RenderList& renderList);

        void HandleMouseEvent(const EV::MouseEvent& event);

        // exported to client
        ENT_Geometry* CreateGeometry(); // thread-safe
        ENT_Text* CreateText();
        ENT_TransformGizmo* CreateTransformGizmo();

        ENT_TransformGizmo* GlobalTransformGizmo();

        // selection, exported to client
        // MUST be called from inside operator
        void                        ClearSelection();
        void 						Select_New(Entity* ent);
        void 						Select_Add(Entity* ent);
        void 						SelectVertex_New(ENT_Geometry* geom, const leda::node vert);
        void 						SelectVertex_Add(ENT_Geometry* geom, const leda::node vert);

        std::vector<ENT_Geometry*>  SelectedGeometry();
        M::Vector3                  GlobalCenterOfSelection();

        Entity*                     FirstSelectedEntity();
        Entity*                     NextSelectedEntity(Entity* ent);

        // thread interface
        DWORD Thread_Func(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
