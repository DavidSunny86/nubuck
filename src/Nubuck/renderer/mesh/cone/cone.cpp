#include "cone.h"

namespace R {

Cone::Cone(float radius, float height, int numSlices, const Color& color) {
    const float twoPi = 2.0f * M::PI;
    const float dSl = 2.0f * M::PI / numSlices; // CCW construction

    // origin is center of cone
    const float halfHeight = 0.5f * height;
    const M::Vector3 upperCenter = M::Vector3(0.0f, 0.0f,  halfHeight);
    const M::Vector3 lowerCenter = M::Vector3(0.0f, 0.0f, -halfHeight);

    Mesh::Vertex vert;
    unsigned indexCnt = 0;
    vert.color = color;
    
    // lower cap
    vert.normal = M::Vector3(0.0f, 0.0f, -1.0f);
    int numVerts = 0;
    float alpha = twoPi;
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

    // mantle
    for(unsigned i = 0; i <= numSlices; ++i) {
        const unsigned idx = i % numSlices;
        // TODO: fix normals
        const M::Vector3 n = M::Vector3(cosf(-dSl * idx), sinf(-dSl * idx), 0.0f);
        vert.normal = n;
        vert.position = lowerCenter + radius * n;
        _vertices.push_back(vert);
        _indices.push_back(indexCnt++);
        vert.position = upperCenter;
        _vertices.push_back(vert);
        _indices.push_back(indexCnt++);
    }
}

Mesh::Desc Cone::GetDesc() {
    Mesh::Desc desc;
    desc.vertices = &_vertices[0];
    desc.numVertices = _vertices.size();
    desc.indices = &_indices[0];
    desc.numIndices = _indices.size();
    desc.primType = GL_TRIANGLE_STRIP;
    return desc;
}

} // namespace R