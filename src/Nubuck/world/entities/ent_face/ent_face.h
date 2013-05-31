#pragma once

#include <world\entity.h>

namespace W {

    class ENT_Face : public Entity {
    private:
        Mesh _mesh;
    public:
        struct SpawnArgs {
            const graph_t*  G;
            leda::edge      edge;
        };

        void Spawn(const Event& event);

        void Render(std::vector<R::RenderJob>& renderList) override;
    };

} // namespace W