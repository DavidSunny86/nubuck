#pragma once

#include <world\entity.h>

namespace W {

    class ENT_Face : public Entity {
    private:
        Mesh _mesh;
        R::SkinMgr::handle_t _skin;
        R::Material _material;
    public:
        struct SpawnArgs {
            const graph_t*  G;
            leda::edge      edge;
            float           r, g, b;
        };

        void Spawn(const Event& event);

        void Render(std::vector<R::RenderJob>& renderList) override;
    };

} // namespace W