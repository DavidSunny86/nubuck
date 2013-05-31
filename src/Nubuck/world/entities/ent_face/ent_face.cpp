#include <renderer\mesh\polygon\polygon.h>
#include "ent_face.h"

namespace W {

    void ENT_Face::Spawn(const Event& event) {
        leda::list<M::Vector2> P;
        /*P.push_back(M::Vector2(-1.0f, -1.0f));
        P.push_back(M::Vector2( 1.0f, -1.0f));
        P.push_back(M::Vector2( 0.0f,  1.0f));*/
        P.push_back(M::Vector2(-2.0f, -2.0f));
        P.push_back(M::Vector2( 0.0f, -3.0f));
        P.push_back(M::Vector2( 1.0f,  2.0f));
        P.push_back(M::Vector2(-1.0f,  3.0f));
        P.push_back(M::Vector2(-3.0f,  1.0f));
        for(int i = 0; i < 3; ++i)
            P = R::ChaikinSubdiv(P);

        R::PolygonMesh polyDesc(P);
        _mesh = MeshFromDesc(polyDesc.GetSolidDesc());
    }

    void ENT_Face::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        renderJob.vertices  = _mesh.vertices;
        renderJob.indices   = _mesh.indices;
        renderJob.primType  = _mesh.primType;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = GetMaterial();

        // solid
        renderJob.fx = "Lit";
        renderList.push_back(renderJob);

        // wireframe
        renderJob.fx = "GenericWireframe";
        renderList.push_back(renderJob);
    }

} // namespace W