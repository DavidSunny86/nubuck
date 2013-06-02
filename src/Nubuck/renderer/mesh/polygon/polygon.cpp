#include <assert.h>
#include <algorithm>
#include <math\line2.h>
#include "polygon.h"

namespace R {

    M::Vector2 Normal(const M::Vector2 v) {
        return M::Normalize(M::Vector2(-v.y, v.x));
    }

    void Subdiv(leda::list<M::Vector2>& polygon) {
        leda::list_item next, it = polygon.first();
        while(NULL != it) {
            next = polygon.succ(it);
            const M::Vector2& v0 = polygon[polygon.cyclic_succ(it)];
            const M::Vector2& v1 = polygon[it];
            polygon.insert(0.5f * (v0 + v1), it, leda::behind);
            it = next;
        }
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

    PolygonMesh::PolygonMesh(const leda::list<M::Vector2>& polygon, const M::Vector3& normal) {
        const float size = 0.5f;
        bool loop = true;
        
        Vertex vert;
        vert.normal = normal;
        vert.color = Color::White;

        unsigned indexCnt = 0;

        float texCoord = 0.0f;
        M::Vector2 lastVec = polygon.front();

        leda::list_item it = polygon.first();
        while(NULL != it) {
            const M::Vector2& v = polygon[it];
            const M::Vector2& n = polygon[polygon.cyclic_succ(it)];
            const M::Vector2& p = polygon[polygon.cyclic_pred(it)];

            M::Vector2 v1 = v - size * Normal(v - p);
            M::Vector2 v2 = v - size * Normal(n - v);
            
            texCoord += M::Distance(lastVec, v);

            vert.position = M::Vector3(v.x, v.y, 0.0f);
            vert.texCoords = M::Vector2(texCoord, 0.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert.position = M::Vector3(v1.x, v1.y, 0.0f);
            vert.texCoords = M::Vector2(texCoord, 1.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert.position = M::Vector3(v.x, v.y, 0.0f);
            vert.texCoords = M::Vector2(texCoord, 0.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert.position = M::Vector3(v2.x, v2.y, 0.0f);
            vert.texCoords = M::Vector2(texCoord, 1.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            lastVec = v;
            it = polygon.succ(it);
        }

        if(loop) {
            _indices.push_back(0);
            _indices.push_back(1);
        }
    }

    template<typename TYPE> // TYPE in {Matrix3, Matrix4}
    struct TransformFunc {
        const TYPE& mat;
        
        TransformFunc(const TYPE& mat) : mat(mat) { }

        void operator()(Vertex& vert) {
            vert.position = M::Transform(mat, vert.position);
        }
    };

    void PolygonMesh::Transform(const M::Matrix3& mat) {
        std::for_each(_vertices.begin(), _vertices.end(), TransformFunc<M::Matrix3>(mat));
    }

    void PolygonMesh::Transform(const M::Matrix4& mat) {
        std::for_each(_vertices.begin(), _vertices.end(), TransformFunc<M::Matrix4>(mat));
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