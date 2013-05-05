#include <common\common.h>
#include <world\entities\ent_node\ent_node.h>
#include "polyhedron.h"
#include "entity.h"
#include "world.h"

namespace W {

    World world;

    World::World(void) : _entIdCnt(0), _secsPassed(0.0f), _timePassed(0.0f) { }

    World::entPtr_t World::GetEntityById(int id) {
        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            if(id == (*entIt)->GetID()) return *entIt;
        }
        return entPtr_t();
    }

    void World::Accept(const Visitor& visitor) {
        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            entPtr_t& entPtr = *entIt;
            visitor.Visit(*entPtr);
        }
    }

    void World::Add(const entPtr_t& entity) {
        _entities.push_back(entity);
    }

    void World::Send(const Event& event) {
        _eventsLock.Lock();
        _events.push(event);
        _eventsLock.Unlock();
    }

    int World::Spawn(Event event) {
        int id = ++_entIdCnt;

        event.id        = EVENT_SPAWN_ENTITY;
        event.entityId  = id;
        Send(event);

        return id;
    }

    void World::Update(void) {
        _secsPassed = _timer.Stop();
        _timePassed += _secsPassed;
        _timer.Start();

        while(!_events.empty()) {
            _eventsLock.Lock();
            Event event = _events.front();
            _events.pop();
            _eventsLock.Unlock();

            if(EVENT_SPAWN_ENTITY == event.id) {
                entAlloc_t alloc = _entityAllocs[EntityType(event.type)];
                GEN::Pointer<Entity> entity(alloc());
                entity->Spawn(event);
                _entities.push_back(entity);
            }

            if(0 < event.entityId) {
                entPtr_t ent = GetEntityById(event.entityId);
                if(ent.IsValid()) ent->HandleEvent(event);
            } else {
                // broadcast
                for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt)
                    (*entIt)->HandleEvent(event);
            }
        }

        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            (*entIt)->Update(_secsPassed);
        }
    }

    void World::FreeResources(void) {
        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            (*entIt)->FreeResources();
        }
    }

    void World::Apocalypse(void) {
        FreeResources();
        _entities.clear();
    }

    IPolyhedron* World::CreatePolyhedron(const graph_t& G) {
        return new Polyhedron(G);
    }

    DWORD World::Run(void) {
        while(true) {
            Update();
            Sleep(10);
        }
    }

} // namespace W
