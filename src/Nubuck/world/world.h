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
#include <math\vector2.h>
#include <math\ray.h>
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

    class World : public IWorld, public SYS::Thread, public EV::EventHandler<World> {
        DECLARE_EVENT_HANDLER(World)
    private:
        struct Selection {
            IGeometry*   geometry;
            Selection() : geometry(NULL) { }
        } _selection;

        std::vector<GEN::Pointer<Entity> > _entities;
        SYS::SpinLock _entitiesMtx;

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        void SetupLights(R::RenderList& renderList);

        ArcballCamera _camArcball;

        GEN::Pointer<Entity> FindByEntityID(unsigned entId);

        void GetInfo(Entity& ent, EntityInf& inf);

        bool        _isGrabbing;
        M::Vector3  _grabPivot;

#pragma region EventHandlers
        void Event_Apocalypse(const EV::Event& event);
        void Event_LinkEntity(const EV::Event& event);
        void Event_SpawnPolyhedron(const EV::Event& event);
        void Event_SpawnMesh(const EV::Event& event);
        void Event_DestroyEntity(const EV::Event& event);
        void Event_Rebuild(const EV::Event& event);
        void Event_SetVisible(const EV::Event& event);
        void Event_SetName(const EV::Event& event);
        void Event_SetPosition(const EV::Event& event);
        void Event_SetScale(const EV::Event& event);
        void Event_SetRotation(const EV::Event& event);
        void Event_SetRenderFlags(const EV::Event& event);
        void Event_SetPickable(const EV::Event& event);
        void Event_SetNodeColor(const EV::Event& event);
        void Event_SetEdgeColor(const EV::Event& event);
        void Event_SetFaceColor(const EV::Event& event);
        void Event_SetFaceVisibility(const EV::Event& event);
        void Event_SetHullAlpha(const EV::Event& event);
        void Event_SetEdgeBaseColor(const EV::Event& event);
        void Event_SetEdgeRadius(const EV::Event& event);
        void Event_SetEffect(const EV::Event& event);
        void Event_Resize(const EV::Event& event);
        void Event_Mouse(const EV::Event& event);
        void Event_Key(const EV::Event& event);
// region EventHandlers
#pragma endregion
    public:
		World(void);

        M::Matrix4 GetCameraMatrix() const { return _camArcball.GetWorldMatrix(); }
        M::Matrix4 GetModelView() const { return _camArcball.GetWorldMatrix(); }

        M::Ray PickingRay(const M::Vector2& mouseCoords);

		unsigned SpawnPolyhedron(graph_t* const G, leda::node_map<bool>* cachedNodes, leda::edge_map<bool>* cachedEdges);
        unsigned SpawnMesh(R::meshPtr_t meshPtr);

        void Update(void);
        void Render(R::RenderList& renderList);

        // exported to client
        IPolyhedron* CreatePolyhedron(void) override;
        IGeometry* CreateGeometry() override;
        IMesh* CreatePlaneMesh(const PlaneDesc& desc) override;
        IMesh* CreateSphereMesh(const SphereDesc& desc) override;
        IMesh* CreateCylinderMesh(const CylinderDesc& desc) override;

        void SelectGeometry(IGeometry* geom) override { _selection.geometry = geom; }
        IGeometry* SelectedGeometry() override { return _selection.geometry; }

        // thread interface
        DWORD Thread_Func(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
