#include <functional>
#include <algorithm>

#include <common\common.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "polyhedron.h"
#include "entity.h"
#include "world.h"

namespace W {

    World world;

    World::handle_t World::NewHandle(void) {
        _entMapMtx.Lock();
        handle_t h = INVALID_HANDLE;
        unsigned i = 0;
        while(INVALID_HANDLE == h && i < _entMap.size()) {
            if(!_entMap[i].used) {
                _entMap[i].used = true;
                h = i;
            }
            ++i;
        }
        if(INVALID_HANDLE == h) {
            h = _entMap.size();
            EntMapItem item;
            item.entIdx = EntMapItem::INVALID_INDEX;
            item.used = true;
            _entMap.push_back(item);
        }
        _entMapMtx.Unlock();
        return h;
    }

	void World::AddRenderJobs(const ENT_Polyhedron& polyhedron) {
		_renderList.jobs.insert(_renderList.jobs.end(),
			polyhedron.renderList.begin(),
			polyhedron.renderList.end());
	}

    void World::SetupLights(void) {
        R::Light light;

        float dist = 20;

        light.constantAttenuation   = 1.0f;
        light.linearAttenuation     = 0.01f;
        light.quadricAttenuation    = 0.0f;

        light.position          = M::Vector3(-dist,  dist, dist);
        // light.diffuseColor      = R::Color::Red;
        light.diffuseColor = R::Color(0.8f, 0.8f, 0.8f);
        _renderList.lights.push_back(light);

        light.position          = M::Vector3( dist,  dist, dist);
        // light.diffuseColor      = R::Color::White;
        light.diffuseColor = R::Color(0.8f, 0.8f, 0.8f);
        _renderList.lights.push_back(light);

        light.position          = M::Vector3( dist, -dist, dist);
        // light.diffuseColor      = R::Color::Blue;
        light.diffuseColor = R::Color(0.8f, 0.8f, 0.8f);
        _renderList.lights.push_back(light);
    }

	World::World(void) {
        SetupLights();
	}

    void World::Send(const Event& event) {
        _eventsMtx.Lock();
        _events.push(event);
        _eventsMtx.Unlock();
    }

	unsigned World::SpawnPolyhedron(const graph_t* const G) {
        handle_t h = NewHandle();
        Event event;
        event.type = EVENT_SPAWN_POLYHEDRON;
        EvArgs_SpawnPolyhedron* args = (EvArgs_SpawnPolyhedron*)event.args;
        args->h = h;
        args->G = G;
        Send(event);
        return h;
	}

    void World::Update(void) {
        _secsPassed = _timer.Stop();
        _timePassed += _secsPassed;
        _timer.Start();

        bool done = false;
        while(!done) {
            Event event;
            _eventsMtx.Lock();
            if(_events.empty()) done = true;
            else {
                event = _events.front();
                _events.pop();
            }
            _eventsMtx.Unlock();
            if(done) break;

            if(EVENT_SPAWN_POLYHEDRON == event.type) {
                EvArgs_SpawnPolyhedron* args = (EvArgs_SpawnPolyhedron*)event.args;
                ENT_Polyhedron ph;
                ph.G = args->G;
                Polyhedron_Init(ph);
                Polyhedron_Rebuild(ph);
                Polyhedron_Update(ph);
                _entMap[args->h].entIdx = _polyhedrons.size();
                _polyhedrons.push_back(ph);
            }

            if(EVENT_REBUILD == event.type) {
                EvArgs_Rebuild* args = (EvArgs_Rebuild*)event.args;
                ENT_Polyhedron& ph = _polyhedrons[_entMap[args->entId].entIdx];
                Polyhedron_Rebuild(ph);
                Polyhedron_Update(ph);
            }

            if(EVENT_SET_NODE_COLOR == event.type) {
                EvArgs_SetNodeColor* args = (EvArgs_SetNodeColor*)event.args;
                ENT_Polyhedron& ph = _polyhedrons[_entMap[args->entId].entIdx];
                ph.nodes.colors[args->node->id()] = args->color;
            }

            if(EVENT_SET_FACE_COLOR == event.type) {
                EvArgs_SetFaceColor* args = (EvArgs_SetFaceColor*)event.args;
                ENT_Polyhedron& ph = _polyhedrons[_entMap[args->entId].entIdx];
                /*
                PolyhedronHullFaceList& face = ph.hull.faceLists[ph.hull.edges[args->edge->id()].faceIdx];
                for(unsigned i = 0; i < face.size; ++i) {
                    ph.hull.vertices[face.base + i].color = args->color;
                }
                Polyhedron_Update(ph);
                */
                Polyhedron_AddCurve(ph, args->edge);
            }
        } // while(!done)

        for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
            for(unsigned j = 0; j < _polyhedrons[i].hull.curves.size(); ++j)
                Polyhedron_UpdateCurve(_polyhedrons[i].hull.curves[j], _secsPassed);
        }

        std::for_each(_polyhedrons.begin(), _polyhedrons.end(), Polyhedron_BuildRenderList);

		_renderListLock.Lock();
        _renderList.worldMat = M::Mat4::Translate(0.0f, 0.0f, -20.0f);
		_renderList.jobs.clear();
		std::for_each(_polyhedrons.begin(), _polyhedrons.end(),
			std::bind(&World::AddRenderJobs, this, std::placeholders::_1));
		_renderListLock.Unlock();
    }

    void World::CopyRenderList(R::RenderList& renderList) {
        _renderListLock.Lock();
        renderList = _renderList;
        _renderListLock.Unlock();
    }

    IPolyhedron* World::CreatePolyhedron(const graph_t& G) {
        return new Polyhedron(G);
    }

    DWORD World::Thread_Func(void) {
        Polyhedron_InitResources();
        while(true) {
            Update();
            Sleep(10);
        }
    }

} // namespace W
