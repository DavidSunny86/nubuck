#include <common\common.h>
#include <world\entities\ent_node\ent_node.h>
#include "polyhedron.h"
#include "entity.h"
#include "world.h"

namespace W {

    World world;

    World::World(void) : _entIdCnt(0), _secsPassed(0.0f), _timePassed(0.0f), _numVisitors(0) { }

    World::entPtr_t World::GetEntityById(int id) {
        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            if(id == (*entIt)->GetID()) return *entIt;
        }
        return entPtr_t();
    }

    void World::Accept(const Visitor& visitor) {
        _numVisitorsLock.Lock();
        if(1 == ++_numVisitors) _entitiesLock.Lock();
        _numVisitorsLock.Unlock();

        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            entPtr_t& entPtr = *entIt;
            visitor.Visit(*entPtr);
        }

        _numVisitorsLock.Lock();
        if(0 == --_numVisitors) _entitiesLock.Unlock();
        _numVisitorsLock.Unlock();
    }

    void World::Send(const Event& event) {
        _eventsLock.Lock();
        _events.push(event);
        _eventsLock.Unlock();
#ifdef NUBUCK_MT
        // TODO: why does the deadlock only occur in release mode?
        if(event.sem) event.sem->Wait();
#endif
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

            if(EVENT_APOCALYPSE == event.id) {
                _entitiesLock.Lock();
                _entities.clear();
                _entitiesLock.Unlock();
                continue;
            }

            if(EVENT_SPAWN_ENTITY == event.id) {
                entAlloc_t alloc = _entityAllocs[EntityType(event.type)];
                GEN::Pointer<Entity> entity(alloc());
                entity->Spawn(event);
                _entitiesLock.Lock();
                _entities.push_back(entity);
                _entitiesLock.Unlock();
            }
            if(EVENT_DESTROY_ENTITY == event.id) {
                entPtr_t ent = GetEntityById(event.entityId);
                if(ent.IsValid()) ent->Destroy();
            }

            if(0 < event.entityId) {
                entPtr_t ent = GetEntityById(event.entityId);
                if(ent.IsValid()) ent->HandleEvent(event);
            } else {
                // broadcast
                for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt)
                    (*entIt)->HandleEvent(event);
            }

#ifdef NUBUCK_MT
            if(event.sem) event.sem->Signal();
#endif
        }

        _entitiesLock.Lock();
        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            if((*entIt)->IsDead()) {
                entIt = _entities.erase(entIt);
                if(_entities.end() == entIt) break;
            }
            else (*entIt)->Update(_secsPassed);
        }
        _entitiesLock.Unlock();

        _renderListLock.Lock();
        _renderList.clear();
        for(entIt_t entIt(_entities.begin()); _entities.end() != entIt; ++entIt) {
            (*entIt)->Render(_renderList);
        }
        _renderListLock.Unlock();
    }

    void World::CopyRenderList(std::vector<R::RenderJob>& renderList) {
        _renderListLock.Lock();
        renderList = _renderList;
        _renderListLock.Unlock();
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
