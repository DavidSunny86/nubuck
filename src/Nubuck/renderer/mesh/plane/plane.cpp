#include "plane.h"

namespace R {

static bool IsValidGridPos(int i, int j, int N) {
    return 0 <= i && i < N && 0 <= j && j < N;
}

// the planes faces the positive z axis. set flip=true to face the negative z axis.
Plane::Plane(int subdiv, float size, heightFunc_t heightFunc, bool flip) {
    int N = 1 + (1 << subdiv);

    Mesh::Vertex defaultVert;
    defaultVert.color = Color::White;
    defaultVert.normal = M::Vector3::Zero;

    // compute vertex positions

    // arranges vertices on NxN grid, centered around it's origin
    _vertices.resize(N * N, defaultVert);
    std::vector<int> numTris(N * N, 0);
    const float halfSize = 0.5f * size;
    const float segSize = size / (N - 1);
    for(unsigned i = 0; i < N; ++i) {
        for(unsigned j = 0; j < N; ++j) {
            Mesh::Vertex& vert = _vertices[i * N + j];
            vert.position.x = segSize * j - halfSize;
            vert.position.y = segSize * i - halfSize;
            vert.position.z = heightFunc(vert.position.x, vert.position.y);
        }
    }

    // compute normals

    // every vertex is incident to at most six triangles. adjTri lists adjacent vertices,
    // encoded by offsets of rows and columns.
    AdjVert adjTri[] = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 } };
    for(unsigned i = 0; i < N; ++i) {
        for(unsigned j = 0; j < N; ++j) {
            for(unsigned k = 0; k < 6; ++k) {
                AdjVert a0 = adjTri[k];
                AdjVert a1 = adjTri[(k + 1) % 6];
                bool c0 = IsValidGridPos(i + a0.drow, j + a0.dcol, N);
                bool c1 = IsValidGridPos(i + a1.drow, j + a1.dcol, N);
                if(c0 && c1) {
                    M::Vector3 v0 = _vertices[i * N + j].position;
                    M::Vector3 v1 = _vertices[(i + a0.drow) * N + (j + a0.dcol)].position;
                    M::Vector3 v2 = _vertices[(i + a1.drow) * N + (j + a1.dcol)].position;
                    M::Vector3 normal = M::Normalize(M::Cross(v1 - v0, v2 - v0));
                    _vertices[i * N + j].normal += normal;
                    numTris[i * N + j]++;
                }
            }
        }
    }
    for(unsigned i = 0; i < N; ++i) {
        for(unsigned j = 0; j < N; ++j) {
            Mesh::Vertex& vert = _vertices[i * N + j];
            vert.normal = M::Normalize(vert.normal / numTris[i * N + j]);
            if(flip) vert.normal *= -1.0f;
        }
    }

    // compute indices

    for(unsigned j = 0; j < N - 1; ++j) {
        for(unsigned i = 0; i < N; ++i) {
            if(flip) {
                _indices.push_back(i * N + j + 1);
                _indices.push_back(i * N + j);
            }
            else {
                _indices.push_back(i * N + j);
                _indices.push_back(i * N + j + 1);
            }
        }
        _indices.push_back(Mesh::RESTART_INDEX);
    }
}

Mesh::Desc Plane::GetDesc(void) {
    Mesh::Desc desc;
    desc.vertices       = &_vertices[0];
    desc.numVertices    = _vertices.size();
    desc.indices        = &_indices[0];
    desc.numIndices     = _indices.size();
    desc.primType       = GL_TRIANGLE_STRIP;
    return desc;
}

} // namespace R