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

    void MakeObtuse(leda::list<M::Vector3>& polygon) {
        leda::list_item it;

        M::Vector3 center = M::Vector3::Zero;
        forall_items(it, polygon) center += polygon[it];
        center /= polygon.size();

        forall_items(it, polygon) {
            M::Vector3& v = polygon[it];
            v = 0.7f * (v - center) + center;
        }

        forall_items(it, polygon) {
            M::Vector3& v = polygon[it];
            M::Vector3 e0 = M::Normalize(polygon[polygon.cyclic_succ(it)] - v);
            M::Vector3 e1 = M::Normalize(polygon[polygon.cyclic_pred(it)] - v);
            const M::Vector3 d = M::Normalize(0.5f * (e0 + e1));
            int i = 0;
            while(M::Dot(e0, e1) > 0.8f && i < 200) {
                v += 0.2f * d;
                e0 = M::Normalize(polygon[polygon.cyclic_succ(it)] - v);
                e1 = M::Normalize(polygon[polygon.cyclic_pred(it)] - v);
                i++;
            }
        }
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
        MakeObtuse(points);

        const M::Vector3& p0 = points[points.get_item(0)];
        const M::Vector3& p1 = points[points.get_item(1)];
        const M::Vector3& p2 = points[points.get_item(2)];

        M::Vector3 v0 = M::Normalize(p1 - p0);
        M::Vector3 v1 = M::Normalize(p2 - p0);
        M::Vector3 v2 = M::Normalize(M::Cross(v0, v1));

        for(int i = 0; i < 1; ++i) R::Subdiv(points);
        for(int i = 0; i < 4; ++i) points = R::ChaikinSubdiv(points);

        R::PolygonMesh polyMesh(points, v2);

        const float eps = 0.001f; // resolves z-fighting of faces and hull
        polyMesh.Transform(M::Mat4::Translate(eps * v2));

        _mesh = MeshFromDesc(polyMesh.GetSolidDesc());

        R::SkinDesc skinDesc;
        skinDesc.diffuseTexture = "C:\\Libraries\\LEDA\\LEDA-6.4\\res\\Textures\\dot.tga";
        _skin = R::skinMgr.Create(skinDesc);

        _material.diffuseColor = R::Color(spawnArgs->r, spawnArgs->g, spawnArgs->b);
    }

    void ENT_Face::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        renderJob.vertices  = _mesh.vertices;
        renderJob.indices   = _mesh.indices;
        renderJob.primType  = _mesh.primType;
        renderJob.skin      = _skin;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = _material;

        // solid
        renderJob.fx = "FaceRing";
        renderList.push_back(renderJob);

        // wireframe
        renderJob.fx = "GenericWireframe";
        renderJob.skin = R::SkinMgr::handle_t();
        // renderList.push_back(renderJob);
    }

} // namespace W