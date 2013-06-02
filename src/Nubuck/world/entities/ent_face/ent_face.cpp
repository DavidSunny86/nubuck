#include <common\common.h>
#include <math\matrix3.h>
#include <renderer\mesh\polygon\polygon.h>
#include <renderer\skin\skinmgr.h>
#include <world\events.h>
#include "ent_face.h"

namespace {

    M::Vector3 ToVector(const leda::d3_rat_point& p) {
        const leda::d3_point fp = p.to_float();
        return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
    }

} // unnamed namespace

namespace W {

    void ENT_Face::Spawn(const Event& event) {
        Entity::Spawn(event);

        const SpawnArgs* spawnArgs = (const SpawnArgs*)event.args;
        const graph_t& G = *spawnArgs->G;
        leda::edge edge = spawnArgs->edge;

        leda::list<M::Vector3> points;
        leda::edge it = edge;
        do {
            points.push_back(ToVector(G[source(it)]));
        } while(edge != (it = G.face_cycle_succ(it)));
        assert(3 <= points.size());

        const M::Vector3& p0 = points[points.get_item(0)];
        const M::Vector3& p1 = points[points.get_item(1)];
        const M::Vector3& p2 = points[points.get_item(2)];

        M::Vector3 v0 = M::Normalize(p1 - p0);
        M::Vector3 v1 = M::Normalize(p2 - p0);
        M::Vector3 v2 = M::Normalize(M::Cross(v0, v1));

        M::Matrix3 M(M::Mat3::FromColumns(v0, v1, v2));
        if(M::AlmostEqual(0.0f, M::Det(M))) {
            common.printf("ERROR - ENT_Face: non-invertable matrix M.\n");
        }

        // M does not need to be orthogonal
        M::Matrix3 invM(M::Inverse(M));

        leda::list<M::Vector2> lpoly;
        M::Vector3 p;
        forall(p, points) {
            M::Vector3 l = M::Transform(invM, p - p0);
            assert(M::AlmostEqual(0.0f, l.z));
            lpoly.push_back(M::Vector2(l.x, l.y));
        }
        for(int i = 0; i < 4; ++i)
            lpoly = R::ChaikinSubdiv(lpoly);

        R::PolygonMesh polyMesh(lpoly, v2);
        polyMesh.Transform(M);

        const float eps = 0.001f; // resolves z-fighting of faces and hull
        polyMesh.Transform(M::Mat4::Translate(p0 + eps * v2));

        _mesh = MeshFromDesc(polyMesh.GetSolidDesc());

        R::SkinDesc skinDesc;
        skinDesc.diffuseTexture = "C:\\Libraries\\LEDA\\LEDA-6.4\\res\\Textures\\dot.tga";
        _skin = R::skinMgr.Create(skinDesc);
    }

    void ENT_Face::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        renderJob.vertices  = _mesh.vertices;
        renderJob.indices   = _mesh.indices;
        renderJob.primType  = _mesh.primType;
        renderJob.skin      = _skin;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = GetMaterial();

        // solid
        renderJob.fx = "TexDiffuse";
        renderList.push_back(renderJob);
    }

} // namespace W