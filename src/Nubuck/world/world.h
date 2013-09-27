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
#include "events.h"

namespace W {

    extern SYS::Semaphore g_worldSem;

    class World : public IWorld, public SYS::Thread {
    private:
        std::queue<Event> _events;
        SYS::SpinLock _eventsMtx;

        // clients use handles to reference entities.
        // _entMap maps handles to entities.
        struct EntMapItem {
            enum { INVALID_INDEX = UINT_MAX };
            unsigned    entIdx; // indexes _polyhedrons
            bool        used;
        };
        std::vector<EntMapItem> _entMap;
        SYS::SpinLock _entMapMtx;
        enum { INVALID_HANDLE = UINT_MAX };
        typedef unsigned handle_t;

        handle_t NewHandle(void);

		std::vector<ENT_Polyhedron> _polyhedrons;

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        void SetupLights(void);

        ArcballCamera _camArcball;

        void HandleMouseEvent(const Event& event);
    public:
		World(void);

        // message passing
        void Send(const Event& event);

		unsigned SpawnPolyhedron(const graph_t* const G);

        void Update(void);

        // exported to client
        IPolyhedron* CreatePolyhedron(const graph_t& G) override;

        // thread interface
        DWORD Thread_Func(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
