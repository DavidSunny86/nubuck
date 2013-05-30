#include <renderer\mesh\polygon\polygon.h>
#include "ent_face.h"

namespace W {

    void ENT_Face::Spawn(const Event& event) {
        leda::list<M::Vector2> P;
        P.push_back(M::Vector2(-1.0f, -1.0f));
        P.push_back(M::Vector2( 1.0f, -1.0f));
        P.push_back(M::Vector2( 0.0f,  1.0f));

        R::PolygonMesh polyDesc(P);
        Mesh mesh = MeshFromDesc(polyDesc.GetSolidDesc());
        SetMesh(mesh);
    }

} // namespace W