#include <assert.h>
#include <common\common.h> // TODO: removeme
#include <algorithm>
#include <math\line2.h>
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

    PolygonMesh::PolygonMesh(const leda::list<M::Vector2>& polygon, const M::Vector3& normal) {
        const float size = 0.2f;
        bool loop = true;
        
        Vertex vert;
        vert.normal = normal;
        vert.color = Color::White;
        
        float len = 0.0f;

        unsigned indexCnt = 0;

        leda::list_item it = polygon.first();
        while(NULL != it) {
            const M::Vector2& v = polygon[it];
            const M::Vector2& n = polygon[polygon.cyclic_succ(it)];
            const M::Vector2& p = polygon[polygon.cyclic_pred(it)];

            M::Vector2 n0 = size * Normal(v - p);
            M::Vector2 n1 = size * Normal(n - v);

            M::Vector2 v1;
            bool is = M::Intersect(
                M::Line2::FromPoints(v + n0, p + n0),
                M::Line2::FromPoints(v + n1, n + n1),
                &v1);
            assert(is);

            vert.position = M::Vector3(v.x, v.y, 0.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert.position = M::Vector3(v1.x, v1.y, 0.0f);
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            it = polygon.succ(it);

            if(it) len += M::Length(n - v);
        }

        if(loop) {
            vert = _vertices[0];
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);

            vert = _vertices[1];
            _vertices.push_back(vert);
            _indices.push_back(indexCnt++);
        }

        float ds = (float)((int)len) / ((_vertices.size() - 2) >> 1);
        for(unsigned i = 0, j = 0; i < _vertices.size(); i += 2, j++) {
            float texCoord = j * ds;
            _vertices[i].texCoords = M::Vector2(texCoord, 0.0f);
            _vertices[i + 1].texCoords = M::Vector2(texCoord, 1.0f);
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