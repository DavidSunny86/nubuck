#include <renderer\mesh\quad\quad.h>
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
        _mesh = R::meshMgr.Create(R::CreateQuadDesc(0.05f));

        R::SkinDesc skinDesc;
        skinDesc.diffuseTexture = "C:\\Libraries\\LEDA\\LEDA-6.4\\res\\Textures\\circle.tga";
        _skin = R::skinMgr.Create(skinDesc);
    }

    void ENT_Dummy::Update(float secsPassed) {
        _bezierCurve->Update(secsPassed);
        _bezierCurve->SampleEquidistantPoints(0.1f, _decalPos);
    }

    void ENT_Dummy::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob rjob;
        rjob.fx = "TexDiffuse";
        rjob.mesh = _mesh;
        rjob.skin = _skin;
        rjob.primType = 0;
        rjob.material = GetMaterial();

        for(unsigned i = 0; i < _decalPos.size(); ++i) {
            const M::Vector2& p = _decalPos[i];
            rjob.transform = M::Mat4::Translate(GetPosition() + M::Vector3(p.x, p.y, 0.0f));
            renderList.push_back(rjob);
        }
    }

} // namespace W