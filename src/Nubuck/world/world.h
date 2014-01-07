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
#include <renderer\renderer.h>
#include <camera\arcball_camera.h>
#include <events\events.h>

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
    public:
        enum EntityType {
            ENT_POLYHEDRON  = 0,
            ENT_MESH,
            ENT_GEOMETRY
        };
    private:
        struct Transform {
            M::Vector3  position;
            M::Vector3  scale;
            M::Matrix4  rotation;
        };

        struct Entity {
            EntityType      type;
            unsigned        entId;
            std::string     name;
            std::string     fxName;
            Transform       transform;

            ENT_Polyhedron* polyhedron;
            ENT_Mesh*       mesh;
            ENT_Geometry*   geometry;
        };

        std::vector<GEN::Pointer<Entity> > _entities;
        SYS::SpinLock _entitiesMtx;

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        void SetupLights(void);

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

		unsigned SpawnPolyhedron(graph_t* const G, leda::node_map<bool>* cachedNodes, leda::edge_map<bool>* cachedEdges);
        unsigned SpawnMesh(R::MeshMgr::meshPtr_t meshPtr);

        void Update(void);

        // exported to client
        IPolyhedron* CreatePolyhedron(void) override;
        IGeometry* CreateGeometry() override;
        IMesh* CreatePlaneMesh(const PlaneDesc& desc) override;
        IMesh* CreateSphereMesh(const SphereDesc& desc) override;
        IMesh* CreateCylinderMesh(const CylinderDesc& desc) override;

        // thread interface
        DWORD Thread_Func(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
