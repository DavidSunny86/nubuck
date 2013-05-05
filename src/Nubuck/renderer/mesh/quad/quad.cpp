#include "quad.h"

namespace R {

    MeshDesc CreateQuad(float edgeLength) {
        MeshDesc desc;

        Vertex* v = new Vertex[4];
        for(int j = 0; j < 4; ++j) {
            v[j].normal = M::Vector3(0.0f, 0.0f, 1.0f);
            v[j].color = Color::White;
        }
        const float size = 0.5f * edgeLength;
        v[0].position = M::Vector3(-size, -size, 0.0f);
        v[1].position = M::Vector3( size, -size, 0.0f);
        v[2].position = M::Vector3( size,  size, 0.0f);
        v[3].position = M::Vector3(-size,  size, 0.0f);
        desc.vertices       = v;
        desc.numVertices    = 4;

        Index indices[] = { 0, 1, 2, 0, 2, 3 };
        Index* i = new Index[6];
        for(int j = 0; j < 6; ++j)  i[j] = indices[j];
        desc.indices    = i;
        desc.numIndices = 6;

        desc.primType = GL_TRIANGLES;
        
        return desc;
    }

} // namespace R
