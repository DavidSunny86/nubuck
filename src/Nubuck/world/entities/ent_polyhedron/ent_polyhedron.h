#pragma once

#include <common\types.h>
#include <world\entity.h>
#include <renderer\renderer.h>
#include <renderer\mesh\polyhedron\polyhedron.h>

namespace W {

    class ENT_Polyhedron : public Entity {
    private:
        GEN::Pointer<R::PolyhedronMesh> _polyDesc;
        const graph_t* _G;
    public:
        struct SpawnArgs {
            const graph_t* G;
        };

        void Spawn(const Event& event);

        void HandleEvent(const Event& event) override;
    };

} // namespace W