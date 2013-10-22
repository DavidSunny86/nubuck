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
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include <world\entities\ent_mesh\ent_mesh.h>
#include <world\events\events.h>

#pragma region EventDefinitions

BEGIN_EVENT_DEF(Apocalypse)
END_EVENT_DEF

BEGIN_EVENT_DEF(SpawnPolyhedron)
    unsigned    entId;
    graph_t*    G;
END_EVENT_DEF

BEGIN_EVENT_DEF(SpawnMesh)
    unsigned                entId;
    R::MeshMgr::meshPtr_t   meshPtr;
END_EVENT_DEF

BEGIN_EVENT_DEF(DestroyEntity)
    unsigned    entId;
END_EVENT_DEF

BEGIN_EVENT_DEF(Rebuild)
    unsigned    entId;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetVisible)
    unsigned    entId;
    bool        isVisible;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetRenderFlags)
    unsigned    entId;
    int         flags;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetPickable)
    unsigned    entId;
    bool        isPickable;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetNodeColor)
    unsigned    entId;
    leda::node  node;
    R::Color    color;
END_EVENT_DEF

BEGIN_EVENT_DEF(SetFaceColor)
    unsigned    entId;
    leda::edge  edge;
    R::Color    color;
END_EVENT_DEF

BEGIN_EVENT_DEF(Resize)
    int         width;
    int         height;
END_EVENT_DEF

BEGIN_EVENT_DEF(Mouse)
    enum Type { MOUSE_DOWN, MOUSE_UP, MOUSE_WHEEL, MOUSE_MOVE };
    enum Button { 
        BUTTON_LEFT     = 1, // == Qt::LeftButton
        BUTTON_RIGHT    = 2, // == Qt::RightButton
        BUTTON_MIDDLE 
    };
    enum Modifier {
        MODIFIER_SHIFT = 0x02000000 // == Qt::ShiftModifier
    };
    int type, button, mods, delta, x, y;
END_EVENT_DEF

BEGIN_EVENT_DEF(Key)
    enum Type { KEY_DOWN = 0, KEY_UP };
    int type;
    int keyCode;
    bool autoRepeat;
END_EVENT_DEF

// region EventDefintions
#pragma endregion

namespace W {

    extern SYS::Semaphore g_worldSem;

    class World : public IWorld, public SYS::Thread, public EV::EventHandler<World> {
        DECLARE_EVENT_HANDLER(World)
    private:
        enum EntityType {
            ENT_POLYHEDRON  = 0,
            ENT_MESH
        };

        struct Entity {
            EntityType      type;
            unsigned        entId;

            ENT_Polyhedron* polyhedron;
            ENT_Mesh*       mesh;
        };

        std::vector<GEN::Pointer<Entity> > _entities;

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        void SetupLights(void);

        ArcballCamera _camArcball;

        GEN::Pointer<Entity> FindByEntityID(unsigned entId);

        bool        _isGrabbing;
        M::Vector3  _grabPivot;

#pragma region EventHandlers
        void Event_Apocalypse(const EV::Event& event);
        void Event_SpawnPolyhedron(const EV::Event& event);
        void Event_SpawnMesh(const EV::Event& event);
        void Event_DestroyEntity(const EV::Event& event);
        void Event_Rebuild(const EV::Event& event);
        void Event_SetVisible(const EV::Event& event);
        void Event_SetRenderFlags(const EV::Event& event);
        void Event_SetPickable(const EV::Event& event);
        void Event_SetNodeColor(const EV::Event& event);
        void Event_SetFaceColor(const EV::Event& event);
        void Event_Resize(const EV::Event& event);
        void Event_Mouse(const EV::Event& event);
        void Event_Key(const EV::Event& event);
// region EventHandlers
#pragma endregion
    public:
		World(void);

		unsigned SpawnPolyhedron(graph_t* const G);
        unsigned SpawnMesh(R::MeshMgr::meshPtr_t meshPtr);

        void Update(void);

        // exported to client
        IPolyhedron* CreatePolyhedron(graph_t& G) override;
        IMesh* CreatePlaneMesh(int subdiv, float size, planeHeightFunc_t heightFunc, bool flip) override;

        // thread interface
        DWORD Thread_Func(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
