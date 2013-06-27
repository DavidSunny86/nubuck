#include <assert.h>
#include <algorithm>
#include <math\line2.h>
#include "polygon.h"

namespace R {

    M::Vector2 Normal(const M::Vector2 v) {
        return M::Normalize(M::Vector2(-v.y, v.x));
    }

    PolygonMesh::PolygonMesh(const leda::list<M::Vector2>& polygon, const M::Vector3& normal) {
        const float size = 0.5f;
        bool loop = true;
        
        Mesh::Vertex vert;
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

    PolygonMesh::PolygonMesh(leda::list<M::Vector3>& polygon, const M::Vector3& normal) {
        const float size = 0.2f;
        bool loop = true;
        
        Mesh::Vertex vert;
        vert.normal = normal;
        vert.color = Color::White;

        unsigned indexCnt = 0;

        float texCoord = 0.0f;
        M::Vector3 lastVec = polygon.front();

        leda::list_item it = polygon.first();
        while(NULL != it) {
            const M::Vector3& v = polygon[it];
            const M::Vector3& n = polygon[polygon.cyclic_succ(it)];
            const M::Vector3& p = polygon[polygon.cyclic_pred(it)];

            M::Vector3 n0 = M::Normalize(M::Cross(v - p, normal));
            M::Vector3 n1 = M::Normalize(M::Cross(n - v, normal));
            M::Vector3 v1 = v + size * M::Normalize(0.5 * (n0 + n1));
            
            texCoord += M::Distance(lastVec, v);

            vert.position = v;
            vert.texCoords = M::Vector2(texCoord, 0.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert.position = v1;
            vert.texCoords = M::Vector2(texCoord, 1.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            lastVec = v;
            it = polygon.succ(it);
        }

        if(loop) {
            texCoord += M::Distance(lastVec, _vertices[0].position);
            texCoord = (int)texCoord;

            vert = _vertices[0];
            vert.texCoords.s = texCoord;
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert = _vertices[1];
            vert.texCoords.s = texCoord;
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);
        }
    }

    template<typename TYPE> // TYPE in {Matrix3, Matrix4}
    struct TransformFunc {
        const TYPE& mat;
        
        TransformFunc(const TYPE& mat) : mat(mat) { }

        void operator()(Mesh::Vertex& vert) {
            vert.position = M::Transform(mat, vert.position);
        }
    };

    void PolygonMesh::Transform(const M::Matrix3& mat) {
        std::for_each(_vertices.begin(), _vertices.end(), TransformFunc<M::Matrix3>(mat));
    }

    void PolygonMesh::Transform(const M::Matrix4& mat) {
        std::for_each(_vertices.begin(), _vertices.end(), TransformFunc<M::Matrix4>(mat));
    }

    Mesh::Desc PolygonMesh::GetDesc(void) {
        Mesh::Desc desc;

        desc.vertices = &_vertices[0];
        desc.numVertices = _vertices.size();
        desc.indices = &_indices[0];
        desc.numIndices = _indices.size();
        desc.primType = GL_TRIANGLE_STRIP;

        return desc;
    }

} // namespace R