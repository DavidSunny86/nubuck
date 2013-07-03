#include "ent_dummy.h"

namespace W {

    ENT_Dummy::ENT_Dummy(void) {
        const M::Vector2 c0(-1.0f, -1.0f);
        const M::Vector2 c1( 1.0f, -1.0f);
        const M::Vector2 c2( 0.0f,  1.0f);
        const float lf = 0.2f, ln = (1.0f - lf);
        std::vector<M::Vector2> points;
        points.push_back(ln * c0 + lf * c1);
        points.push_back(0.5f * c0 + 0.5f * c1);
        points.push_back(lf * c0 + ln * c1);
        points.push_back(c1);
        points.push_back(ln * c1 + lf * c2);
        points.push_back(0.5f * c1 + 0.5f * c2);
        points.push_back(lf * c1 + ln * c2);
        points.push_back(c2);
        points.push_back(ln * c2 + lf * c0);
        points.push_back(0.5f * c2 + 0.5f * c0);
        points.push_back(lf * c2 + ln * c0);
        points.push_back(c0);
        points.push_back(ln * c0 + lf * c1);
        _bezierCurve = GEN::Pointer<R::PolyBezier2U>(new R::PolyBezier2U(points));
        _mesh = R::meshMgr.Create(_bezierCurve->GetDesc());
        _material.diffuseColor = R::Color::Black;
    }

    void ENT_Dummy::Update(float secsPassed) {
        _bezierCurve->Update(secsPassed);
        _mesh->Invalidate(_bezierCurve->GetDesc().vertices);
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