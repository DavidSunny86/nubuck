#pragma once

#include <common\types.h>
#include <common\config\config.h>
#include <world\entity.h>

extern COM::Config::Variable<float> cvar_nodeSize;
extern COM::Config::Variable<int> cvar_nodeSubdiv;

namespace W {

    class ENT_Node : public Entity {
    private:
        leda::node _node;

        struct ConfigObserver : COM::Config::Observer {
            void CVAR_Changed(const std::string& name) override;
        };
        static ConfigObserver s_configObs;

        static R::meshPtr_t s_mesh;
        static R::Material s_material;
        static void CreateMesh(void);
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