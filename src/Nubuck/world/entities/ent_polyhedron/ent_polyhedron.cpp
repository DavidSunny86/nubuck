#include <common\common.h>
#include <world\events.h>
#include <renderer\mesh\quad\quad.h>
#include "ent_polyhedron.h"

namespace W {

    void ENT_Polyhedron::Spawn(const Event& event) {
        Entity::Spawn(event);

        const SpawnArgs* spawnArgs = (const SpawnArgs*)event.args;
        _G = spawnArgs->G;

        if(!spawnArgs->G->empty())
            _mesh = GEN::Pointer<R::PolyhedronMesh>(new R::PolyhedronMesh(*_G));
    }

    void ENT_Polyhedron::HandleEvent(const Event& event) {
        if(EVENT_REBUILD == event.id) {
            R::PolyhedronMesh polyMesh = R::PolyhedronMesh(*_G);
            SetMeshHandle(R::MeshMgr::Instance().CreateMesh(polyMesh.GetDesc()));
        }
    }

} // namespace W