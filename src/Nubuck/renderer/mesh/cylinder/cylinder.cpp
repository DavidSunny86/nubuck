#include "cylinder.h"

namespace {

	M::Vector3 PointOnCircle(float delta, unsigned i) {
		return M::Vector3(cosf(delta * i), sinf(delta * i), 0.0f);
	}

    R::Mesh::Vertex CreateVertex(
        const M::Vector3& position, 
        const M::Vector3& normal,
        const R::Color& color = R::Color::White)
    {
        R::Mesh::Vertex vertex;
        vertex.position = position;
        vertex.normal = normal;
        vertex.color = color;
        return vertex;
    }

} // unnamed namespace

namespace R {

    // TODO: texture coordinates
	Cylinder::Cylinder(float radius, float height, unsigned numSlices, bool caps) {
        const float twoPi = 2.0f * M::PI;
		const float dSl = 2.0f * M::PI / numSlices; // CCW construction

        // origin is center of cylinder
        const float halfHeight = 0.5f * height;
		const M::Vector3 upperCenter = M::Vector3(0.0f, 0.0f,  halfHeight);
		const M::Vector3 lowerCenter = M::Vector3(0.0f, 0.0f, -halfHeight);

        Mesh::Vertex vert;
        vert.color = Color::White;
		unsigned indexCnt = 0;

        if(caps) {
            unsigned numVerts;
            float alpha;

            // upper cap
            vert.normal = M::Vector3(0.0f, 0.0f, 1.0f);
            numVerts = 0;
            alpha = 0.0f;
            while(numSlices > numVerts) {
                M::Vector3 n;
                if(!(numVerts % 2)) n = M::Vector3(cosf(twoPi - alpha), sinf(twoPi - alpha), 0.0f);
                else n = M::Vector3(cosf(alpha), sinf(alpha), 0.0f);
                vert.position = upperCenter + radius * n;
                _vertices.push_back(vert);
                _indices.push_back(indexCnt++);
                if(!(numVerts % 2)) alpha += dSl;
                numVerts++;
            }
            _indices.push_back(Mesh::RESTART_INDEX);

            // lower cap
            vert.normal = M::Vector3(0.0f, 0.0f, -1.0f);
            numVerts = 0;
            alpha = twoPi;
            while(numSlices > numVerts) {
                M::Vector3 n;
                if(!(numVerts % 2)) n = M::Vector3(cosf(twoPi - alpha), sinf(twoPi - alpha), 0.0f);
                else n = M::Vector3(cosf(alpha), sinf(alpha), 0.0f);
                vert.position = lowerCenter + radius * n;
                _vertices.push_back(vert);
                _indices.push_back(indexCnt++);
                if(!(numVerts % 2)) alpha -= dSl;
                numVerts++;
            }
            _indices.push_back(Mesh::RESTART_INDEX);
        } // if(caps)

        // mantle
		for(unsigned i = 0; i <= numSlices; ++i) {
            const unsigned idx = i % numSlices;
			const M::Vector3 n = M::Vector3(cosf(dSl * idx), sinf(dSl * idx), 0.0f);
            vert.normal = n;
            vert.position = lowerCenter + radius * n;
            _vertices.push_back(vert);
			_indices.push_back(indexCnt++);
            vert.position = upperCenter + radius * n;
			_vertices.push_back(vert);
			_indices.push_back(indexCnt++);
		}
	}

    Mesh::Desc Cylinder::GetDesc(void) {
        Mesh::Desc desc;
        desc.vertices = &_vertices[0];
        desc.numVertices = _vertices.size();
        desc.indices = &_indices[0];
        desc.numIndices = _indices.size();
        desc.primType = GL_TRIANGLE_STRIP;
        return desc;
    }

} // namespace R