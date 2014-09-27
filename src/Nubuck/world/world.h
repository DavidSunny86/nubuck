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
#include <world\editmode\editmode.h>
#include "entity.h"

namespace W {

    class ENT_Polyhedron;
    class ENT_Mesh;
    class ENT_Geometry;
    class ENT_Text;

    extern SYS::Semaphore g_worldSem;

    struct EntityInf {
        unsigned    entId;
        std::string name;
        void*       inf;
    };

    class World : public SYS::Thread, public EV::EventHandler<> {
    private:
        DECL_HANDLE_EVENTS(World)

        class Selection {
        private:
            mutable SYS::SpinLock _mtx;

            std::vector<Entity*>    list;
            M::Vector3              center; // in world space

            void ComputeCenter();
            void SignalChange();
        public:
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

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        void SetupLights(R::RenderList& renderList);

        ArcballCamera _camArcball;

        GEN::Pointer<Entity> FindByEntityID(unsigned entId);

        R::meshPtr_t    _gridMesh;
        R::tfmeshPtr_t  _gridTFMesh;

        void Grid_Build();
        void Grid_GetRenderJobs(std::vector<R::MeshJob>& rjobs);

        struct BoundingBox {
            const ENT_Geometry*     geom;
            R::meshPtr_t            mesh;
            R::tfmeshPtr_t          tfmesh;

            void Destroy();

            BoundingBox() : geom(NULL), mesh(NULL), tfmesh(NULL) { }
            BoundingBox(const ENT_Geometry* geom);
            ~BoundingBox() { Destroy(); }
            void Transform();
        };
        std::vector<GEN::Pointer<BoundingBox> > _bboxes;
        void                        BBoxes_BuildFromSelection();
        void                        BBoxes_GetRenderJobs(std::vector<R::MeshJob>& rjobs);

        EditMode _editMode;

#pragma region EventHandlers
        void Event_Apocalypse(const EV::Event& event);
        void Event_LinkEntity(const EV::Event& event);
        void Event_DestroyEntity(const EV::Event& event);
        void Event_SelectionChanged(const EV::Event& event);
        void Event_RebuildAll(const EV::Event& event);
        void Event_EditModeChanged(const EV::Event& event);
        void Event_Resize(const EV::Event& event);
        void Event_Mouse(const EV::Event& event);
        void Event_Key(const EV::Event& event);
// region EventHandlers
#pragma endregion
    public:
		World(void);

        const EditMode& GetEditMode() const { return _editMode; }
        EditMode&       GetEditMode() { return _editMode; };

        ArcballCamera&  GetCamera() { return _camArcball; }

        M::Matrix4 GetCameraMatrix() const { return _camArcball.GetWorldToEyeMatrix(); }
        M::Matrix4 GetModelView() const { return _camArcball.GetWorldToEyeMatrix(); }

        M::Ray  PickingRay(const M::Vector2& mouseCoords);
        bool    Trace(const M::Ray& ray, ENT_Geometry** geom);

        void RebuildAll();
        void Update(void);
        void Render(R::RenderList& renderList);

        // exported to client
        Selection* GetSelection();
        ENT_Geometry* CreateGeometry(); // thread-safe
        ENT_Text* CreateText();

        // thread interface
        DWORD Thread_Func(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
