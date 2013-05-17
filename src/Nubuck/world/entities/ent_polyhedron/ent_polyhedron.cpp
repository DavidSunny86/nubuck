#include <common\common.h>
#include <world\events.h>
#include <renderer\mesh\quad\quad.h>
#include "ent_polyhedron.h"

namespace W {

    void ENT_Polyhedron::Spawn(const Event& event) {
        Entity::Spawn(event);

        const SpawnArgs* spawnArgs = (const SpawnArgs*)event.args;
        _G = spawnArgs->G;

        /*
        NOTE: ignore this for now...
        if(!spawnArgs->G->empty())
            _polyDesc = GEN::Pointer<R::PolyhedronMesh>(new R::PolyhedronMesh(*_G));
        */
    }

    void ENT_Polyhedron::HandleEvent(const Event& event) {
        if(EVENT_REBUILD == event.id) {
            _polyDesc = GEN::Pointer<R::PolyhedronMesh>(new R::PolyhedronMesh(*_G));
            SetMeshHandle(R::MeshMgr::Instance().CreateMesh(_polyDesc));
        }
        if(EVENT_CHANGE_COLOR == event.id) {
            ChangeColorArgs* args = (ChangeColorArgs*)event.args;
            R::Color color(args->r, args->g, args->b);
            _polyDesc->Set(args->edge, color);
            InvalidateMesh();
        }
    }

} // namespace W