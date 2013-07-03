#pragma once

#include <world\entity.h>

#include <generic\pointer.h>
#include <renderer\mesh\bezier\bezier.h>

namespace W {

    class ENT_Dummy : public Entity {
    private:
        GEN::Pointer<R::PolyBezier2U> _bezierCurve;
        R::MeshMgr::MeshPtr _mesh;
        R::SkinMgr::handle_t _skin;

        std::vector<M::Vector2> _decalPos;
    public:
        ENT_Dummy(void);

        void Update(float secsPassed) override;
        void Render(std::vector<R::RenderJob>& renderList) override;
    };

} // namespace W