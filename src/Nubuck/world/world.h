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
#include "events.h"

namespace W {

    enum EntityType {
        ENT_NODE = 0,
        ENT_POLYHEDRON
    };

    class Entity;

    class World : public IWorld, public SYS::Thread {
    private:
        typedef GEN::Pointer<Entity>                entPtr_t;
        typedef std::vector<entPtr_t>::iterator     entIt_t;

        typedef Entity* (*entAlloc_t)(void);
        std::map<EntityType, entAlloc_t> _entityAllocs;

        std::vector<entPtr_t>   _entities;
        SYS::SpinLock           _entitiesLock;
        int                     _entIdCnt;

        std::queue<Event> _events;
        SYS::SpinLock _eventsLock;

        SYS::Timer  _timer;
        float       _secsPassed;
        float       _timePassed;

        int             _numVisitors;
        SYS::SpinLock   _numVisitorsLock;

        std::vector<R::RenderJob>   _renderList;
        SYS::SpinLock               _renderListLock;

        entPtr_t GetEntityById(int id);
    public:
        struct Visitor {
            virtual ~Visitor(void) { }

            virtual void Visit(Entity& entity) const = 0;
        };

        World(void);

        void Accept(const Visitor& visitor);

        template<typename TYPE> 
        void RegisterEntity(EntityType type);
        
        void Add(const entPtr_t& entity);

        // message passing
        void Send(const Event& event);

        int Spawn(Event event);

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
