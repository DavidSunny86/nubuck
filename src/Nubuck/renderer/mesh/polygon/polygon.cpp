#include "polygon.h"

namespace R {

    M::Vector2 Normal(const M::Vector2 v) {
        return M::Normalize(M::Vector2(-v.y, v.x));
    }

    leda::list<M::Vector2> ChaikinSubdiv(const leda::list<M::Vector2>& polygon) {
        leda::list<M::Vector2> refined;
        leda::list_item it = polygon.first();
        while(NULL != it) {
            const M::Vector2& v0 = polygon[polygon.cyclic_pred(it)];
            const M::Vector2& v1 = polygon[it];

            refined.push_back(0.75 * v0 + 0.25 * v1);
            refined.push_back(0.25 * v0 + 0.75 * v1);

            it = polygon.succ(it);
        }

        return refined;
    }

    PolygonMesh::PolygonMesh(const leda::list<M::Vector2>& polygon) {
        const float size = 0.2f;
        bool loop = true;
        
        Vertex vert;
        vert.normal = M::Vector3(0.0f, 0.0f, 1.0f);
        vert.color = Color::White;
        
        unsigned indexCnt = 0;

        leda::list_item it = polygon.first();
        while(NULL != it) {
            const M::Vector2& v = polygon[it];
            const M::Vector2& n = polygon[polygon.cyclic_succ(it)];
            const M::Vector2& p = polygon[polygon.cyclic_pred(it)];

            M::Vector2 d = M::Normalize(0.5f * (Normal(n - v) + Normal(v - p)));
            M::Vector2 v0 = v - size * d;
            M::Vector2 v1 = v + size * d;

            vert.position = M::Vector3(v0.x, v0.y, 0.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert.position = M::Vector3(v1.x, v1.y, 0.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            it = polygon.succ(it);
        }

        if(loop) {
            _indices.push_back(0);
            _indices.push_back(1);
        }
    }

    MeshDesc PolygonMesh::GetSolidDesc(void) {
        MeshDesc desc;

        desc.vertices = &_vertices[0];
        desc.numVertices = _vertices.size();
        desc.indices = &_indices[0];
        desc.numIndices = _indices.size();
        desc.primType = GL_TRIANGLE_STRIP;

        return desc;
    }

} // namespace R