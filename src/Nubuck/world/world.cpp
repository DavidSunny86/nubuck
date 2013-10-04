#include <functional>
#include <algorithm>

#include <common\common.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "polyhedron.h"
#include "entity.h"
#include "world.h"

namespace W {
    
    SYS::Semaphore g_worldSem(0);
    static int rlIdx = 1;

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

	static void AddRenderJobs(const ENT_Polyhedron& polyhedron) {
        R::RenderList& _renderList = R::g_renderLists[rlIdx];
		_renderList.jobs.insert(_renderList.jobs.end(),
			polyhedron.renderList.jobs.begin(),
			polyhedron.renderList.jobs.end());
        _renderList.nodePositions.insert(_renderList.nodePositions.end(),
            polyhedron.renderList.nodePositions.begin(),
            polyhedron.renderList.nodePositions.end());
        _renderList.edges.insert(_renderList.edges.end(),
            polyhedron.renderList.edges.begin(),
            polyhedron.renderList.edges.end());
	}

    void World::SetupLights(void) {
        R::Light light;

        R::RenderList& _renderList = R::g_renderLists[rlIdx];
        _renderList.lights.clear();

        _renderList.dirLights[0].direction      = M::Vector3(-1.0f,  1.0f,  0.0f);
        _renderList.dirLights[0].diffuseColor   = R::Color(0.6f, 0.85f, 0.91f);

        _renderList.dirLights[1].direction      = M::Vector3( 1.0f,  1.0f,  0.0f);
        _renderList.dirLights[1].diffuseColor   = R::Color(1.0f, 0.5f, 0.15f);

        _renderList.dirLights[2].direction      = M::Vector3( 0.0f, -1.0f, -1.0f);
        _renderList.dirLights[2].diffuseColor   = R::Color(1.0f, 1.0f, 1.0f);

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

    void World::HandleMouseEvent(const Event& event) {
        EvArgs_Mouse* args = (EvArgs_Mouse*)event.args;

        if(EvArgs_Mouse::MOUSE_DOWN == args->type) {
            if(EvArgs_Mouse::BUTTON_LEFT == args->button)
                _camArcball.StartDragging(args->x, args->y);
            if(EvArgs_Mouse::BUTTON_RIGHT == args->button)
                _camArcball.StartPanning(args->x, args->y);
        }

        if(EvArgs_Mouse::MOUSE_UP == args->type) {
            if(EvArgs_Mouse::BUTTON_LEFT  == args->button)
                _camArcball.StopDragging();
            if(EvArgs_Mouse::BUTTON_RIGHT == args->button)
                _camArcball.StopPanning();
        }

        if(EvArgs_Mouse::MOUSE_MOVE == args->type) {
            _camArcball.Drag(args->x, args->y);
            _camArcball.Pan(args->x, args->y);
        }

        if(EvArgs_Mouse::MOUSE_WHEEL == args->type) {
            if(args->delta > 0) _camArcball.ZoomIn();
            if(args->delta < 0) _camArcball.ZoomOut();
        }
    }

    void World::DestroyPolyhedron(handle_t handle) {
        std::swap(_polyhedrons[_entMap[handle].entIdx], _polyhedrons.back());
        _polyhedrons.erase(_polyhedrons.end() - 1);
    }

	World::World(void) : _camArcball(800, 400) /* init values arbitrary */ {
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

        SetupLights();

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

            if(EVENT_DESTROY_POLYHEDRON == event.type) {
                EvArgs_DestroyPolyhedron* args = (EvArgs_DestroyPolyhedron*)event.args;
                DestroyPolyhedron(args->entId);
                _entMap[args->entId].entIdx = EntMapItem::INVALID_INDEX;
                _entMap[args->entId].used = false;
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
                Polyhedron_AddCurve(ph, args->edge, args->color);
            }

            if(EVENT_RESIZE == event.type) {
                EvArgs_Resize* args = (EvArgs_Resize*)event.args;
                _camArcball.SetScreenSize(args->width, args->height);
            }

            if(EVENT_MOUSE == event.type) HandleMouseEvent(event);
        } // while(!done)

        for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
            for(unsigned j = 0; j < _polyhedrons[i].hull.curves.size(); ++j)
                Polyhedron_UpdateCurve(_polyhedrons[i].hull.curves[j], _secsPassed);
        }

        std::for_each(_polyhedrons.begin(), _polyhedrons.end(), Polyhedron_BuildRenderList);

        R::RenderList& _renderList = R::g_renderLists[rlIdx];
        _renderList.worldMat = _camArcball.GetWorldMatrix();
		_renderList.jobs.clear();
        _renderList.nodePositions.clear();
        _renderList.edges.clear();
		std::for_each(_polyhedrons.begin(), _polyhedrons.end(),
			std::bind(AddRenderJobs, std::placeholders::_1));
        g_worldSem.Signal();
        R::g_rendererSem.Wait();
        rlIdx = 1 - rlIdx;
    }

    IPolyhedron* World::CreatePolyhedron(const graph_t& G) {
        return new Polyhedron(G);
    }

    DWORD World::Thread_Func(void) {
        Polyhedron_InitResources();
        while(true) {
            Update();
            // Sleep(10);
        }
    }

} // namespace W
