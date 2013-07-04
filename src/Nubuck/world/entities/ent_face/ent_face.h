#pragma once

#include <common\types.h>
#include <common\config\config.h>
#include <math\matrix3.h>
#include <world\entity.h>
#include <renderer\mesh\bezier\bezier.h>

namespace W {

    extern COM::Config::Variable<float> cvar_faceDecalSize;

    class ENT_Face : public Entity, public COM::Config::Observer {
    private:
        R::meshPtr_t _mesh;
        R::SkinMgr::handle_t _skin;
        R::Material _material;
        GEN::Pointer<R::PolyBezier2U> _polyBezier;

        std::vector<M::Vector2> _decalPos2;
        std::vector<M::Vector3> _decalPos;

        M::Vector3 _p0; // origin
        M::Vector3 _normal;
        M::Matrix3 _M;

        void Rebuild(void);
        void CreateMesh(void);
    public:
        struct SpawnArgs {
            const graph_t*  G;
            leda::edge      edge;
            float           r, g, b;
        };

        void Spawn(const Event& event);

        void Update(float secsPassed) override;
        void Render(std::vector<R::RenderJob>& renderList) override;

        void CVAR_Changed(const std::string& name) override;
    };

} // namespace W