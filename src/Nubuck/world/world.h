#pragma once

#include <limits>
#include <map>
#include <vector>
#include <queue>

#include <Nubuck\nubuck.h>
#include <generic\pointer.h>
#include <system\timer\timer.h>
#include <system\thread\thread.h>
#include <system\locks\spinlock.h>
#include <system\locks\semaphore.h>
#include <common\types.h>
#include <Nubuck\math\vector2.h>
#include <Nubuck\math\ray.h>
#include <renderer\renderer.h>
#include <camera\arcball_camera.h>
#include <events\events.h>
#include "entity.h"

namespace W {

    class ENT_Polyhedron;
    class ENT_Mesh;
    class ENT_Geometry;

    extern SYS::Semaphore g_worldSem;

    struct EntityInf {
        unsigned    entId;
        std::string name;
        void*       inf;
    };

    class World : public IWorld, public SYS::Thread, public EV::EventHandler<> {
    private:
        DECL_HANDLE_EVENTS(World)

        struct Selection {
            std::vector<IGeometry*> geomList;
            M::Vector3              center; // in world space

            void ComputeCenter();

            void Set(IGeometry* geom);
            void Add(IGeometry* geom);

            const M::Vector3&               GetGlobalCenter() const { return center; }
            const std::vector<IGeometry*>&  GetGeometryList() const { return geomList; }
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

        void        Grid_Build();
        R::MeshJob  Grid_GetRenderJob();

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

#pragma region EventHandlers
        void Event_Apocalypse(const EV::Event& event);
        void Event_LinkEntity(const EV::Event& event);
        void Event_DestroyEntity(const EV::Event& event);
        void Event_Resize(const EV::Event& event);
        void Event_Mouse(const EV::Event& event);
        void Event_Key(const EV::Event& event);
// region EventHandlers
#pragma endregion
    public:
		World(void);

        M::Matrix4 GetCameraMatrix() const { return _camArcball.GetWorldMatrix(); }
        M::Matrix4 GetModelView() const { return _camArcball.GetWorldMatrix(); }

        M::Ray  PickingRay(const M::Vector2& mouseCoords);
        bool    Trace(const M::Ray& ray, ENT_Geometry** geom);

        void Update(void);
        void Render(R::RenderList& renderList);

        // exported to client
        IGeometry* CreateGeometry() override;

        void SelectGeometry(IGeometry* geom) override;
        void AddToSelection(IGeometry* geom);
        void ClearSelection();
        Selection& GetSelection() { return _selection; }

        // thread interface
        DWORD Thread_Func(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
