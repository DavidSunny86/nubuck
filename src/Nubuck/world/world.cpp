#include <functional>
#include <algorithm>

#include <common\common.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "polyhedron.h"
#include "entity.h"
#include "world.h"

namespace W {

    World world;

	void World::AddRenderJobs(const ENT_Polyhedron& polyhedron) {
		_renderList.insert(_renderList.end(),
			polyhedron.renderList.begin(),
			polyhedron.renderList.end());
	}

	World::World(void) {
	}

    void World::Send(const Event& event) {
        _eventsMtx.Lock();
        _events.push(event);
        _eventsMtx.Unlock();
    }

	unsigned World::SpawnPolyhedron(const graph_t* const G) {
        unsigned entId = _polyhedrons.size();
		_polyhedrons.resize(_polyhedrons.size() + 1);
		ENT_Polyhedron& polyhedron = _polyhedrons.back();
		polyhedron.G = G;
		Polyhedron_Init(polyhedron);
		Polyhedron_Rebuild(polyhedron);
		Polyhedron_Update(polyhedron);
		Polyhedron_BuildRenderList(polyhedron);
        return entId;
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

            if(EVENT_REBUILD == event.type) {
                EvArgs_Rebuild* args = (EvArgs_Rebuild*)event.args;
                ENT_Polyhedron& ph = _polyhedrons[args->entId];
                Polyhedron_Rebuild(ph);
                Polyhedron_Update(ph);
            }

            if(EVENT_SET_NODE_COLOR == event.type) {
                EvArgs_SetNodeColor* args = (EvArgs_SetNodeColor*)event.args;
                ENT_Polyhedron& ph = _polyhedrons[args->entId];
                ph.nodes.colors[args->node->id()] = args->color;
            }

            if(EVENT_SET_FACE_COLOR == event.type) {
                EvArgs_SetFaceColor* args = (EvArgs_SetFaceColor*)event.args;
                ENT_Polyhedron& ph = _polyhedrons[args->entId];
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
		_renderList.clear();
		std::for_each(_polyhedrons.begin(), _polyhedrons.end(),
			std::bind(&World::AddRenderJobs, this, std::placeholders::_1));
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
        Polyhedron_InitResources();
        while(true) {
            Update();
            Sleep(10);
        }
    }

} // namespace W
