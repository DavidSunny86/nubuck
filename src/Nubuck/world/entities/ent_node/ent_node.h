#pragma once

#include <common\types.h>
#include <world\entity.h>

namespace W {

    class ENT_Node : public Entity {
    private:
        leda::node _node;

        Mesh _mesh;

        static R::MeshMgr::vertexHandle_t   s_meshVertices;
        static R::MeshMgr::indexHandle_t    s_meshIndices;
        static void InitResources(void);
    public:
        struct SpawnArgs {
            const graph_t*  G;
            leda::node      node;
        };

        void HandleEvent(const Event& event);
        void Spawn(const Event& event) override;
        void Render(std::vector<R::RenderJob>& renderList) override;
    };

} // namespace W