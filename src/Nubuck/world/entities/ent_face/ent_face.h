#pragma once

#include <math\matrix3.h>
#include <world\entity.h>
#include <renderer\mesh\bezier\bezier.h>

namespace W {

    class ENT_Face : public Entity {
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
    public:
        struct SpawnArgs {
            const graph_t*  G;
            leda::edge      edge;
            float           r, g, b;
        };

        void Spawn(const Event& event);

        void Update(float secsPassed) override;
        void Render(std::vector<R::RenderJob>& renderList) override;
    };

} // namespace W