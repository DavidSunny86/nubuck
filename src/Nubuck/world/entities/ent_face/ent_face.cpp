#include <common\common.h>
#include <math\matrix3.h>
#include <renderer\mesh\quad\quad.h>
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

    void Shrink(leda::list<M::Vector3>& points, float f) {
        leda::list_item it;
        M::Vector3 center = M::Vector3::Zero;
        forall_items(it, points) center += points[it];
        center /= points.size();
        forall_items(it, points) points[it] = points[it] - f * (points[it] - center);
    }

    inline void PushTransformed(const M::Matrix3& invM, const M::Vector3& p0, const M::Vector3& p, std::vector<M::Vector2>& poly) {
        M::Vector3 l = M::Transform(invM, p - p0);
        assert(M::AlmostEqual(0.0f, l.z));
        poly.push_back(M::Vector2(l.x, l.y));
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
        Shrink(points, 0.2f);

        const M::Vector3& p0 = points[points.get_item(0)];
        const M::Vector3& p1 = points[points.get_item(1)];
        const M::Vector3& p2 = points[points.get_item(2)];

        M::Vector3 v0 = M::Normalize(p1 - p0);
        M::Vector3 v1 = M::Normalize(p2 - p0);
        M::Vector3 v2 = M::Normalize(M::Cross(v0, v1));

        _M = M::Matrix3(M::Mat3::FromColumns(v0, v1, v2));
        if(M::AlmostEqual(0.0f, M::Det(_M))) common.printf("ERROR - ENT_Face: non-invertable matrix M.\n");
        M::Orthonormalize(_M);
        M::Matrix3 invM(M::Transpose(_M));

        const float s = 0.2f;

        std::vector<M::Vector2> poly;
        leda::list_item pIt = NULL;
        forall_items(pIt, points) {
            const M::Vector3& c0 = points[pIt];
            const M::Vector3& c1 = points[points.cyclic_succ(pIt)];
            PushTransformed(invM, p0, (1.0f - s) * c0 + s * c1, poly); // must start at first endpoint!
            PushTransformed(invM, p0, 0.5f * c0 + 0.5f * c1, poly);
            PushTransformed(invM, p0, s * c0 + (1.0f - s) * c1, poly);
            PushTransformed(invM, p0, c1, poly);
        }
        poly.push_back(poly.front()); // close poly
        _polyBezier = GEN::Pointer<R::PolyBezier2U>(new R::PolyBezier2U(poly));

        _polyBezier->SampleEquidistantPoints(0.4f, _decalPos2);
        for(unsigned i = 0; i < _decalPos2.size(); ++i) {
            M::Vector3 p = M::Transform(_M, M::Vector3(_decalPos2[i].x, _decalPos2[i].y, 0.0f)) + p0;
            const float eps = 0.001f; // resolves z-fighting of faces and hull
            p += eps * v2;
            _decalPos.push_back(p);
        }

        _mesh = R::meshMgr.Create(R::CreateQuadDesc(0.2f));

        R::SkinDesc skinDesc;
        skinDesc.diffuseTexture = "C:\\Libraries\\LEDA\\LEDA-6.4\\res\\Textures\\circle.tga";
        _skin = R::skinMgr.Create(skinDesc);

        _material.diffuseColor = R::Color(spawnArgs->r, spawnArgs->g, spawnArgs->b);
    }

    void ENT_Face::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        renderJob.mesh      = _mesh;
        renderJob.primType  = 0;
        renderJob.skin      = _skin;
        renderJob.material  = _material;

        // solid
        renderJob.fx = "TexDiffuse";
        for(unsigned i = 0; i < _decalPos.size(); ++i) {
            renderJob.transform = M::Mat4::Translate(GetPosition() + _decalPos[i]) * M::Mat4::FromRigidTransform(_M, M::Vector3::Zero);
            renderList.push_back(renderJob);
        }

        // wireframe
        renderJob.fx = "GenericWireframe";
        renderJob.skin = R::SkinMgr::handle_t();
        // renderList.push_back(renderJob);
    }

} // namespace W