#include "ent_dummy.h"

namespace W {

    ENT_Dummy::ENT_Dummy(void) {
        std::vector<M::Vector2> points;
        points.push_back(M::Vector2(-1.0f, 0.0f));
        points.push_back(M::Vector2( 1.0f, 0.0f));
        points.push_back(M::Vector2( 0.0f, 1.0f));
        _bezierCurve = GEN::Pointer<R::Bezier>(new R::Bezier(points));
        _mesh = R::meshMgr.Create(_bezierCurve->GetDesc());
        _material.diffuseColor = R::Color::Black;
    }

    void ENT_Dummy::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob rjob;
        rjob.fx = "Wireframe";
        rjob.mesh = _mesh;
        rjob.primType = 0;
        rjob.transform = M::Mat4::Translate(GetPosition());
        rjob.material = _material;
        renderList.push_back(rjob);
    }

} // namespace W