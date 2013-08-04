#include <common\common.h>
#include <world\entities\ent_node\ent_node.h>
#include "polyhedron.h"
#include "entity.h"
#include "world.h"

namespace W {

    World world;

	World::World(void) { }

    void World::Send(const Event& event) {
    }

    void World::Update(void) {
        _secsPassed = _timer.Stop();
        _timePassed += _secsPassed;
        _timer.Start();
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
