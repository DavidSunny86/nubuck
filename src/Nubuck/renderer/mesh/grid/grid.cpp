#include "grid.h"

namespace R {

Grid::Grid(int subdiv, float size) {
    int regN = 1 + (1 << subdiv);

    const float hsz = 0.5f * size;
    const float dc = size / regN;

    Mesh::Vertex vert;
    vert.normal = M::Vector3(0.0f, 1.0f, 0.0f);
    vert.color = Color::White;

    unsigned idxCnt = 0;
    float c = 0.0f;

    // horizontal lines
    c = -hsz;
    for(unsigned i = 0; i <= regN; ++i) {
        vert.position = M::Vector3(-hsz, 0.0f, c);
        _vertices.push_back(vert);
        vert.position = M::Vector3( hsz, 0.0f, c);
        _vertices.push_back(vert);
        c += dc;
        _indices.push_back(idxCnt++);
        _indices.push_back(idxCnt++);
    }

    // vertical lines
    c = -hsz;
    for(unsigned i = 0; i <= regN; ++i) {
        vert.position = M::Vector3(c, 0.0f, -hsz);
        _vertices.push_back(vert);
        vert.position = M::Vector3(c, 0.0f,  hsz);
        _vertices.push_back(vert);
        c += dc;
        _indices.push_back(idxCnt++);
        _indices.push_back(idxCnt++);
    }
}

Mesh::Desc Grid::GetDesc() {
    Mesh::Desc desc;
    desc.vertices = &_vertices[0];
    desc.numVertices = _vertices.size();
    desc.indices = &_indices[0];
    desc.numIndices = _indices.size();
    desc.primType = GL_LINES;
    return desc;
}

} // namespace R