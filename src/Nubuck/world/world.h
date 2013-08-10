#pragma once

#include <map>
#include <vector>
#include <queue>

#include <Nubuck\nubuck.h>
#include <generic\pointer.h>
#include <system\timer\timer.h>
#include <system\thread\thread.h>
#include <system\locks\spinlock.h>
#include <common\types.h>
#include <renderer\renderer.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "events.h"

namespace W {

    class World : public IWorld, public SYS::Thread {
    private:
        std::queue<Event> _events;
        SYS::SpinLock _eventsMtx;

		std::vector<ENT_Polyhedron> _polyhedrons;

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        std::vector<R::RenderJob>   _renderList;
        SYS::SpinLock               _renderListLock;

		void AddRenderJobs(const ENT_Polyhedron& polyhedron);
    public:
		World(void);

        // message passing
        void Send(const Event& event);

		unsigned SpawnPolyhedron(const graph_t* const G);

        void Update(void);

        void CopyRenderList(std::vector<R::RenderJob>& renderList);

        // exported to client
        IPolyhedron* CreatePolyhedron(const graph_t& G) override;

        // thread interface
        DWORD Run(void);
    };

    extern World world;

} // namespace W

#include "world_inl.h"
